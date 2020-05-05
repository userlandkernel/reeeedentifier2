#!/usr/bin/env python3
import sys
import os
import time
import struct
import usb.core
import usb.util
import random
import binascii
from pyfiglet import Figlet
hacklet = Figlet(font='doom')

CMD_STATUS = struct.pack('q', 0x0902)
CMD_POWERON = struct.pack('q', 0x0102)
CMD_POWEROFF = struct.pack('q', 0x0b02)

class EddieFuzzer(object):

	def __init__(self, eddie = None):
		self.eddie = eddie
		self.cmds = []
		for i in range(0, 0xffff):
			self.cmds.append(struct.pack('q', i))

	def Fuzz(self, start = 0, count = 0x0, step = 0, timeout = 0.5):

		i = start
		if count == 0:
			count = len(self.cmds) 

		print("Fuzzing %d packets " % (count-start))
		while i < count:
			try:
				err = self.eddie.write(self.cmds[i])

				err2 = None
				try:
					err2 = self.eddie.read()
				except Exception as ex:
					pass

				if err2 != None:
					print("PACKET: %d\tWRET: %d\tRRET: %d" % (i, err, err2))
				else:
					print("PACKET: %d\tWRET: %d" % (i, err))

				if timeout:
					time.sleep(timeout)

				i += step

			except Exception as ex:
				print("ERROR")
				continue

		print("Fuzzer done.")



class Edentifier(object):

	def __init__(self, vid=0x0b0c, pid=0x001e, location=0):

		self.dev = None
		self.buffer = []

		print("#] Waiting for ABN AMRO e.dentifier 2...")
		while self.dev == None:
			self.dev = usb.core.find(idVendor=vid, idProduct=pid)

		if self.dev.is_kernel_driver_active(0):
			try:
				self.dev.detach_kernel_driver(0)
			except usb.core.USBError as ex:
				sys.exit("Failed to detach kernel driver.")

	def Reset(self):
		self.dev.reset()

	def Setup(self):

		try:
			self.Reset()
			self.dev.set_configuration()

		except usb.core.USBError as ex:
			sys.exit("Failed configure e.dentifier.")

		try:
			# Get configuration
			cfg = self.dev.get_active_configuration()

			# Select first interface
			intf = cfg[(0,0)]

			# Get input endpoint
			ep = usb.util.find_descriptor(intf, custom_match=\
			lambda e: \
				usb.util.endpoint_direction(e.bEndpointAddress) == \
				usb.util.ENDPOINT_IN)

			# Update instance ep
			self.epin = ep

			# Get output endpoint
			ep = usb.util.find_descriptor(intf, custom_match=\
                        lambda e: \
                                usb.util.endpoint_direction(e.bEndpointAddress) == \
                                usb.util.ENDPOINT_OUT)

			# Update instance ep
			self.epout = ep

		except Exception as ex:
			print(ex)

	def write(self, data):
		return self.epout.write(data, self.epin.wMaxPacketSize)

	def read(self):
		return self.epin.read(self.epin.wMaxPacketSize)

	def PowerOn(self):
		r = self.write(CMD_POWERON)

		while True:
			try:

				data = self.read()
				if data:
					self.buffer += data
				else:
					break

			except Exception as ex:
				break

	def PowerOff(self):
		return self.write(CMD_POWEROFF)

	def Status(self):
		return self.write(CMD_STATUS)


def banner():

	print(hacklet.renderText(" Reeeedentifier2"))
	print("\t\tCreated by S. Voigtlander")
	print("")


def animation_blink(eddie, seconds=0):
	t1 = time.time()
	while True:
		eddie.Setup()
		time.sleep(0.2)
		eddie.Status()
		time.sleep(0.2)
		if time.time() - t1 > seconds:
			break

def animation_loading(eddie, seconds=0):
	t1 = time.time()
	while True:
		eddie.Setup()
		time.sleep(0.3)
		eddie.PowerOn()
		time.sleep(0.3)
		eddie.Status()
		time.sleep(0.3)
		if time.time() - t1 > seconds:
			break


def main():

	banner()

	eddie = Edentifier()

	# Set up the device
	eddie.Setup()

	print('Uploading...')
	animation_loading(eddie, 20)

	print('Installing...')
	animation_blink(eddie, 10)

	print('Done')

#	print(eddie.Status())
#	print(eddie.PowerOn())
#	print(eddie.PowerOff())

	#fuddie = EddieFuzzer(eddie)

	

	#fuddie.Fuzz(start=0x0102, step = 0x0100, timeout=0)



if __name__ == "__main__":
	main()
