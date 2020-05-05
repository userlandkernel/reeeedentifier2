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

	def Fuzz(self, start = 0, count = 0x0, step = 0, timeout = 0):

		i = start
		if count == 0:
			count = 0xffffffff

		print("Fuzzing %d packets " % (count-start))
		while i < count:
			try:
				err = self.eddie.write(struct.pack('q', i))
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
		print("#] Got one: %s" % usb.util.get_string(self.dev, self.dev.iManufacturer))

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
		return self.epout.write(data, self.epout.wMaxPacketSize)

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


class EDFApplication(object):

	def __init__(self):
		self.args = sys.argv

	def DisplayBanner(self):
		print(hacklet.renderText(" Reeeedentifier2"))
		print("\t\tCreated by S. Voigtlander")
		print("")

	def DisplayUsage(self):
		print("\tUsage: ")
		print(" ")
		print("\t\t--reset\tReset the e.dentifier2")
		print("\t\t--loader [seconds] \tDisplay loading animation on e.dentifier2")
		print("\t\t--blink [seconds] \tDisplay blink animation on e.dentifier2")
		print("\t\t--fuzz (start) (end) (step) (delay)\t Fuzz the e.dentifier2")
		#print("\t\t--morse [morsetext]")
		print("")

	def Fuzz(self, s=0, e=0, st=0, t=0):
		print("Fuzzing e.dentifier2...")
		feddie = EddieFuzzer(self.eddie)
		feddie.Fuzz(start=s, count=e, step=st, timeout=t)

	def Run(self):

		self.DisplayBanner()

		if len(self.args) < 2:
			self.DisplayUsage()
			sys.exit(1)

		self.eddie = Edentifier()
		self.eddie.Setup()

		if self.args[1] == "--reset":
			print("Resetting e.dentifier2")
			self.eddie.Reset()
			print("Done")

		elif self.args[1] == "--loader":

			if len(self.args) <= 2:
				self.DisplayUsage()
				sys.exit(1)

			print("Presenting loading animation on e.dentifier2 for %d seconds" % int(self.args[2]))
			animation_loading(self.eddie, int(self.args[2]))
			print("Done")

		elif self.args[1] == "--blink":

			if len(self.args) <= 2:
				self.DisplayUsage()
				sys.exit(1)

			print("Blinking e.dentifier2 for %d seconds..." % int(self.args[2]))
			animation_blink(self.eddie, int(self.args[2]))
			print("Done")

		elif self.args[1] == "--fuzz":

			if len(self.args) <= 5:
				self.DisplayUsage()
				sys.exit(1)

			s = int(self.args[2])
			e = int(self.args[3])
			st = int(self.args[4])
			t = int(self.args[5])

			self.Fuzz(s,e,st,t)

		if self.eddie:
			self.eddie.Reset()

if __name__ == "__main__":
	app = EDFApplication()
	app.Run()
