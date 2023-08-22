#!/usr/bin/env python3

# open_agb_firm gba_db.bin Builder v4.0
# By HTV04
#
# This script parses MAME's "gba.xml" (https://github.com/mamedev/mame/blob/master/hash/gba.xml)
# and converts it to a "gba_db.bin" file for open_agb_firm.
#
# The official gba_db.bin is built using "--dat" and "--csv." The former uses No-Intro's "gba.dat"
# (https://datomatic.no-intro.org/) to verify the SHA-1 and size of each entry. The latter uses
# "gba.csv" to add additional entries and overrides.
#
# Note that, for efficiency, this script does not check for formatting errors and assumes that all
# entries are valid. Errors may occur otherwise.

# MIT License
#
# Copyright (c) 2023 HTV04
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import enum
import sys
import re
import xml.etree.ElementTree
import csv

class Entry:
	def __init__(self):
		self.sha1 = '\x00' * 20
		self.serial = '\x00' * 4
		self.attr = '\x00' * 4

class Database:
	def __init__(self, log):
		class SaveType(enum.IntEnum):
			EEPROM_8K = 0
			EEPROM_8K_2 = 1
			EEPROM_64K = 2
			EEPROM_64K_2 = 3
			FLASH_512K_PSC_RTC = 8
			FLASH_512K_PSC = 9
			FLASH_1M_MRX_RTC = 10
			FLASH_1M_MRX = 11
			SRAM_256K = 14
			NONE = 15

		class XmlData:
			def __init__(self, software):
				def find_name(node, child, name):
					for found in node.findall(child):
						if found.get('name') == name:
							return found
					else:
						return None

				cart = find_name(software, 'part', 'cart')
				serial = find_name(software, 'info', 'serial')
				if serial is not None:
					serial = re.search(r'[A-Z0-9]{4}', serial.get('value'))
				rom = find_name(cart, 'dataarea', 'rom').find('rom')
				size = int(rom.get('size'), base=0)
				slot = find_name(cart, 'feature', 'slot')
				save_type = SaveType.NONE
				if slot is not None:
					match slot.get('value'):
						case 'gba_eeprom_4k' | 'gba_yoshiug' | 'gba_eeprom':
							if size > 0x1000000:
								save_type = SaveType.EEPROM_8K_2
							else:
								save_type = SaveType.EEPROM_8K
						case 'gba_eeprom_64k' | 'gba_boktai':
							if size > 0x1000000:
								save_type = SaveType.EEPROM_64K_2
							else:
								save_type = SaveType.EEPROM_64K
						case 'gba_flash_rtc':
							save_type = SaveType.FLASH_512K_PSC_RTC
						case 'gba_flash' | 'gba_flash_512':
							save_type = SaveType.FLASH_512K_PSC
						case 'gba_flash_1m_rtc':
							save_type = SaveType.FLASH_1M_MRX_RTC
						case 'gba_flash_1m':
							save_type = SaveType.FLASH_1M_MRX
						case 'gba_sram' | 'gba_drilldoz' | 'gba_wariotws':
							save_type = SaveType.SRAM_256K

				self.sha1 = bytes.fromhex(rom.get('sha1'))
				if serial is None:
					self.serial = b'\x00\x00\x00\x00'
				else:
					self.serial = serial.group().encode('ascii')
				self.attr = int(save_type).to_bytes(4, byteorder='little')

		class DatData:
			def __init__(self, dat, xml_data):
				sha1 = xml_data.sha1.hex()

				self.exists = True
				for game in dat.findall('game'):
					rom = game.find('rom')

					if rom.get('sha1') == sha1:
						serial = rom.find('serial')

						self.size = int(rom.get('size'))
						if serial is None:
							self.serial = xml_data.serial
						else:
							self.serial = serial.encode('ascii')

						break
				else:
					self.exists = False

		def get_entry(software, dat):
			entry = Entry()
			xml_data = XmlData(software)

			entry.sha1 = xml_data.sha1
			for test in self.entries:
				if test.sha1 == entry.sha1:
					return 'Duplicate SHA-1 "' + entry.sha1.hex() + '" '
			entry.attr = xml_data.attr

			if dat is None:
				entry.serial = xml_data.serial
			else:
				dat_data = DatData(dat, xml_data)
				if dat_data.exists:
					entry.serial = dat_data.serial
				else:
					return 'SHA-1 "' + entry.sha1.hex() + '" not found in No-Intro DAT'

			if len(entry.sha1) + len(entry.serial) + len(entry.attr) != 28:
				raise Exception('Database size is not a multiple of 28')

			return entry

		self.entries = []

		log('Starting log...\n\n')

		fail_count = 0
		count = 0

		if '--dat' in sys.argv:
			dat = xml.etree.ElementTree.parse('gba.dat').getroot()
			log('Using gba.dat!\n\n')
		else:
			dat = None

		for software in xml.etree.ElementTree.parse('gba.xml').getroot().findall('software'):
			log('Adding "' + software.find('description').text + '"\n')
			ret = get_entry(software, dat)
			if isinstance(ret, Entry):
				count += 1
				self.entries.append(ret)
				log('Successfully added entry: ' + ret.sha1.hex())
			else:
				fail_count += 1
				log('Failed to add entry: ' + ret)
			log('\n\n')

		if '--csv' in sys.argv:
			log('Using gba.csv!\n\n')

			with open('gba.csv') as f:
				for sha1, serial, save_type in list(csv.reader(f))[1:]:
					entry = Entry()
					entry.sha1 = bytes.fromhex(sha1)
					entry.serial = serial.encode('ascii')
					entry.attr = int(SaveType(int(save_type))).to_bytes(4, byteorder='little')

					for i in range(len(self.entries)):
						if self.entries[i].sha1 == entry.sha1:
							self.entries[i] = entry
							log('Duplicate SHA-1 "' + entry.sha1.hex() + '," replaced')
							break
					else:
						count += 1
						self.entries.append(entry)
						log('Added "' + entry.sha1.hex() + '"')
					log('\n')
				log('\n')

		log('Compiled with ' + str(count) + ' entries, ' + str(fail_count) + ' failures.\n')

	def compile(self, out):
		for entry in sorted(self.entries, key=lambda a: int.from_bytes(a.sha1[:8], byteorder='little')):
			out(entry.sha1)
			out(entry.serial)
			out(entry.attr)

if __name__ == '__main__':
	if '--help' in sys.argv:
		print('open_agb_firm gba_db.bin Builder v4.0')
		print('By HTV04')
		print()
		print('Usage: gba-db.py [options]')
		print('  --log: Write log to "gba_db.log"')
		print()
		print('  --dat: Use No-Intro "gba.dat" for verification')
		print('  --csv: Use "gba.csv" for additional entries and overrides')
		print()
		print('  --out [file]: Output to [file] instead of "gba_db.bin"')
		print()
		print('  --help: Display this help message and exit')

		sys.exit(0)

	if '--log' in sys.argv:
		with open('gba_db.log', 'w') as f:
			db = Database(f.write)
	else:
		db = Database(lambda text: None)

	# Compile to gba_db.bin
	out = 'gba_db.bin'
	if '--out' in sys.argv:
		out = sys.argv[sys.argv.index('--out') + 1]
	with open(out, 'wb') as f:
		db.compile(f.write)
