# open_agb_firm gba_db.bin Builder v1.2
# By HTV04

# This script parses MAME's gba.xml (found here: https://github.com/mamedev/mame/blob/master/hash/gba.xml) and converts it to a gba_db.bin file for open_agb_firm.
# This script should work with any updates to MAME's gba.xml, unless something this script expects is changed.

import math

import xml.etree.ElementTree as ET

# Use title, title ID, SHA-1, size, and save type to generate gba_db entry as binary string
def gbadbentry(title, titleid, sha, size, savetype):
    entry = b''
    
    entry += title.encode().ljust(200, b'\x00') # Pad to 200 bytes with null bytes
    entry += titleid.encode()
    entry += bytes.fromhex(sha)
    entry += (int(math.log(size, 2)) << 27 | savetype).to_bytes(4, 'little') # Save type is stored weirdly
    
    return entry # Entry size is 228 bytes in length

if __name__ == '__main__':
    root = ET.parse('gba.xml').getroot()
    
    gbadb = b''
    
    for software in root.findall('software'):
        # Obtain title
        title = software.find('description').text
        
        # Obtain title ID
        titleid = '\x00\x00\x00\x00' # If a title ID can't be found, default to null bytes
        for info in software.findall('info'):
            if info.get('name') == 'serial':
                for i in info.get('value').split('-'): # Hacky script that checks for the first part of the serial that has 4 characters, since serials vary
                    s = i.strip()
                    if len(s) == 4:
                        titleid = s
                        
                        break
                
                break
        
        for part in software.findall('part'):
            if part.get('name') == 'cart':
                # Obtain SHA-1
                for dataarea in part.findall('dataarea'):
                    if dataarea.get('name') == 'rom':
                        size = int(dataarea.get('size'), 0) # Base 0 so int can decide whether string is decimal or hex
                        
                        sha = dataarea.find('rom').get('sha1')
                        
                        break
                
                # Obtain save type
                savetype = 15 # If a save type can't be found or is unknown, set to "SAVE_TYPE_NONE"
                for feature in part.findall('feature'):
                    if feature.get('name') == 'slot':
                        slottype = feature.get('value')
                        if slottype == 'gba_eeprom_4k':
                            savetype = 0 # SAVE_TYPE_EEPROM_8k
                            if size > 16777216: # If greater than 16 MB, change save type
                                savetype += 1 # SAVE_TYPE_EEPROM_8k_2
                        elif slottype == 'gba_eeprom_64k':
                            savetype = 2 # SAVE_TYPE_EEPROM_64k
                            if size > 16777216: # If greater than 16 MB, change save type
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
        
        # Expand gba_db with entry
        gbadb += gbadbentry(title, titleid, sha, size, savetype)
    
    # Create and write to gba_db.bin
    with open('gba_db.bin', 'wb') as f:
        f.write(gbadb)
