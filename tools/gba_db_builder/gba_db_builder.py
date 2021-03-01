# open_agb_firm gba_db.bin Builder v2.5
# By HTV04
# 
# This script parses MAME's gba.xml (found here: https://github.com/mamedev/mame/blob/master/hash/gba.xml) and converts it to a gba_db.bin file for open_agb_firm.
# No-Intro's GBA DAT is also used for filtering and naming (found here: https://datomatic.no-intro.org/). The DAT should be renamed to "gba.dat".
# 
# This script should work with any updates to MAME's gba.xml and the No-Intro DAT, unless something this script expects is changed.

import math
import re
# import sys

import xml.etree.ElementTree as ET

# Use title, serial, SHA-1, size, and save type to generate gba_db entry as binary string
def gbadbentry(a, b, c, d, e):
    entry = []
    
    if len(c) != 40:
        c = '0000000000000000000000000000000000000000'
    cbytes = bytes.fromhex(c)
    
    entry.append(int.from_bytes(cbytes[:8], 'little')) # Sorting key
    entry.append(a.encode().ljust(200, b'\x00')[:200])
    if len(b) != 4 or bool(re.search('[^A-Z0-9]', b)):
        entry.append(b'\x00\x00\x00\x00')
    else:
        entry.append(b.encode())
    entry.append(cbytes)
    entry.append((int(math.log(d, 2)) << 27 | e).to_bytes(4, 'little')) # Save type is stored weirdly
    
    return entry

# Prepare gba_db.bin binary string from gba_db list.
def preparegbadbbin(a):
    gbadbbin = b''
    
    # Use sort key to sort the gba_db list and delete it from each entry
    a = sorted(a, key=lambda l:l[0])
    length = len(a)
    for i in range(length):
        a[i].pop(0)
    
    # Compile gba_db binary
    for i in a:
        for j in i:
            gbadbbin += j
    
    return gbadbbin

if __name__ == '__main__':
    gbadb = []
    skipcount = 0
    count = 0
    addcount = 0
    
    gba = ET.parse('gba.xml').getroot() # MAME gba.xml
    nointro = ET.parse('gba.dat').getroot() # No-Intro GBA DAT
    
    # # Arguments
    # if sys.argv[1] == 'noaddentries': # Don't include additional entries
    #     addentries = False
    addentries = False # Leave as False for now since no additional entries have been added yet
    
    # Start adding entries
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
                        if rom.get('sha1') == sha.upper():
                            matchfound = True
                            
                            title = game.get('name')
                            serial = rom.get('serial')
                            if serial == None:
                                serial = ''
                            size = int(rom.get('size'))
                
                # If not in No-Intro DAT, skip entry
                if matchfound == False:
                    break
                
                # Obtain save type
                savetype = 15 # SAVE_TYPE_NONE
                for feature in part.findall('feature'):
                    if feature.get('name') == 'slot':
                        slottype = feature.get('value')
                        if slottype in ('gba_eeprom', 'gba_eeprom_4k'):
                            savetype = 0 # SAVE_TYPE_EEPROM_8k
                            if size > 0x1000000:
                                savetype += 1 # SAVE_TYPE_EEPROM_8k_2
                        elif slottype == 'gba_eeprom_64k':
                            savetype = 2 # SAVE_TYPE_EEPROM_64k
                            if size > 0x1000000:
                                savetype += 1 # SAVE_TYPE_EEPROM_64k_2
                        elif slottype == 'gba_flash_rtc':
                            savetype = 8 # SAVE_TYPE_FLASH_512k_PSC_RTC
                        elif slottype == 'gba_flash_512':
                            savetype = 9 # SAVE_TYPE_FLASH_512k_PSC
                        elif slottype == 'gba_flash_1m_rtc':
                            savetype = 10 # SAVE_TYPE_FLASH_1m_MRX_RTC
                        elif slottype == 'gba_flash_1m':
                            savetype = 11 # SAVE_TYPE_FLASH_1m_MRX
                        elif slottype == 'gba_sram':
                            savetype = 14 # SAVE_TYPE_SRAM_256k
                        
                        break
        
        # If not in No-Intro DAT, skip entry
        if matchfound == False:
            print ('Skipped "' + software.find('description').text + '"')
            skipcount += 1
            
            continue
        
        # Expand gba_db with entry
        gbadb.append(gbadbentry(title, serial, sha, size, savetype))
        
        print('Added entry "' + software.find('description').text + '"')
        count += 1
    
    # Add additional entries
    if addentries == True:
        # Example entries for demonstration purposes (to do: replace with valid entries)
        gbadbentries = ([['AAAA - A', 'AAAA', 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA', 0x400000, 15],
                         ['BBBB - B', 'BBBB', 'BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB', 0x400000, 15],
                         ['CCCC - C', 'CCCC', 'CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC', 0x400000, 15]])
        
        print()
        
        for title, serial, sha, size, savetype in gbadbentries:
            gbadb.append(gbadbentry(title, serial, sha, size, savetype))
            
            print('Added additional entry "' + title + '"')
            addcount += 1
    
    print('\nFinalizing...\n')
    
    gbadbbin = preparegbadbbin(gbadb)
    
    # Create and write to gba_db.bin
    with open('gba_db.bin', 'wb') as f:
        f.write(gbadbbin)
    
    if addentries == True:
        print(str(count) + ' entries added, ' + str(addcount) + ' additional entries added, ' + str(skipcount) + ' entries skipped')
    else:
        print(str(count) + ' entries added, ' + str(skipcount) + ' entries skipped')
