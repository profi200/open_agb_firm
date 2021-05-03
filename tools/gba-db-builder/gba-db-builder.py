#!/usr/bin/env python3

# open_agb_firm gba_db.bin Builder v3.0
# By HTV04
# 
# This script parses MAME's gba.xml (found here: https://github.com/mamedev/mame/blob/master/hash/gba.xml) and converts it to a gba_db.bin file for open_agb_firm.
# No-Intro's GBA DAT (with scene numbers) is also used for filtering and naming (found here: https://datomatic.no-intro.org/). The DAT should be renamed to "gba.dat".
# Unless otherwise specified, entries from an addentries.csv file are also added. This file usually includes entries that cannot be not found or are wrong in MAME's gba.xml.
# 
# This script should work with any updates to MAME's gba.xml and the No-Intro DAT, unless something this script expects is changed.

import csv
import math
import re
import sys

import xml.etree.ElementTree as ET

# Use title, serial, SHA-1, size, and save type to generate gba_db entry as binary string
def gbadbentry(title, serial, sha, size, savetype):
    entry = []
    
    if len(sha) != 40:
        sha = '0000000000000000000000000000000000000000'
    shabytes = bytes.fromhex(sha)
    
    entry.append(int.from_bytes(shabytes[:8], 'little')) # Sorting key
    entry.append(title.encode().ljust(200, b'\x00')[:200])
    if len(serial) != 4 or bool(re.search('[^A-Z0-9]', serial)):
        entry.append(b'\x00\x00\x00\x00')
    else:
        entry.append(serial.encode())
    entry.append(shabytes)
    entry.append((int(math.log(size, 2)) << 27 | savetype).to_bytes(4, 'little'))
    
    return entry

# Prepare gba_db list for gba_db.bin
def preparegbadb(gbadb):
   # Use sort key to sort the gba_db list and delete it from each entry
    gbadb = sorted(gbadb, key=lambda l:l[0])
    for i in range(len(gbadb)):
        gbadb[i].pop(0)
    
    # Compile gba_db binary
    gbadbbin = b''
    for i in gbadb:
        for j in i:
            gbadbbin += j
    
    return gbadbbin

if __name__ == '__main__':
    # Arguments (could totally be done better but this will do for now)
    noaddentries = False
    if len(sys.argv) >= 2 and sys.argv[1] == 'noaddentries': # Don't include anything that isn't in gba.xml and gba.dat
        noaddentries = True
    
    # Start adding entries
    gbadb = []
    skipcount = 0
    count = 0
    gba = ET.parse('gba.xml').getroot() # MAME gba.xml
    nointro = ET.parse('gba.dat').getroot() # No-Intro GBA DAT
    for software in gba.findall('software'):
        for part in software.findall('part'):
            if part.get('name') == 'cart':
                # Obtain SHA-1
                for dataarea in part.findall('dataarea'):
                    if dataarea.get('name') == 'rom':
                        sha = dataarea.find('rom').get('sha1')
                        
                        break
                
                # Obtain title, serial, SHA-1, and size from No-Intro DAT
                matchfound = False
                for game in nointro.findall('game'):
                    for rom in game.findall('rom'):
                        if rom.get('sha1').lower() == sha:
                            title = game.get('name')
                            serial = rom.get('serial')
                            if serial == None:
                                serial = ''
                            size = int(rom.get('size'))
                            
                            matchfound = True
                            
                            break
                
                # If not in No-Intro DAT, skip entry
                if not matchfound:
                    break
                
                # Obtain save type
                savetype = 15 # SAVE_TYPE_NONE
                for feature in part.findall('feature'):
                    if feature.get('name') == 'slot':
                        slottype = feature.get('value')
                        if slottype in ('gba_eeprom_4k', 'gba_yoshiug', 'gba_eeprom'):
                            savetype = 0 # SAVE_TYPE_EEPROM_8k
                            if size > 0x1000000:
                                savetype += 1 # SAVE_TYPE_EEPROM_8k_2
                        elif slottype in ('gba_eeprom_64k', 'gba_boktai'):
                            savetype = 2 # SAVE_TYPE_EEPROM_64k
                            if size > 0x1000000:
                                savetype += 1 # SAVE_TYPE_EEPROM_64k_2
                        elif slottype == 'gba_flash_rtc':
                            savetype = 8 # SAVE_TYPE_FLASH_512k_PSC_RTC
                        elif slottype in ('gba_flash', 'gba_flash_512'):
                            savetype = 9 # SAVE_TYPE_FLASH_512k_PSC
                        elif slottype == 'gba_flash_1m_rtc':
                            savetype = 10 # SAVE_TYPE_FLASH_1m_MRX_RTC
                        elif slottype == 'gba_flash_1m':
                            savetype = 11 # SAVE_TYPE_FLASH_1m_MRX
                        elif slottype in ('gba_sram', 'gba_drilldoz', 'gba_wariotws'):
                            savetype = 14 # SAVE_TYPE_SRAM_256k
                        
                        break
        
        # If not in No-Intro DAT, skip entry
        if not matchfound:
            print ('Skipped "' + software.find('description').text + '"')
            skipcount += 1
            
            continue
        
        # Add entry to gba_db
        entry = gbadbentry(title, serial, sha, size, savetype)
        for i in range(len(gbadb)):
            if gbadb[i][3].hex() == sha:
                print('Duplicate entry "' +  gbadb[i][1].decode() + '" replaced')
                gbadb[i] = entry
                skipcount += 1
                break
        else:
            gbadb.append(entry)
            count += 1
        
        print('Added entry "' + title + '"')
    
    # Add additional entries from addentries.csv if "noaddentries" is false
    if not noaddentries:
        with open('addentries.csv') as f:
            addentries = list(csv.reader(f))
        
        addentries.pop(0)
        for i in addentries:
            i[3] = int(i[3])
            i[4] = int(i[4])
        
        print()
        
        for title, serial, sha, size, savetype in addentries:
            entry = gbadbentry(title, serial, sha, size, savetype)
            for i in range(len(gbadb)):
                if gbadb[i][3].hex().upper() == sha:
                    print('Duplicate entry "' +  gbadb[i][1].decode() + '" replaced')
                    gbadb[i] = entry
                    skipcount += 1
                    break
            else:
                gbadb.append(entry)
                count += 1
            
            print('Added additional entry "' + title + '"')
    
    gbadbbin = preparegbadb(gbadb)
    
    # Create and write to gba_db.bin
    with open('gba_db.bin', 'wb') as f:
        f.write(gbadbbin)
    
    print('\n' + str(count) + ' entries added, ' + str(skipcount) + ' entries skipped')
