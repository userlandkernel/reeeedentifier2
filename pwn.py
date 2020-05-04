#!/usr/bin/env python
import usb.core
import usb.util
import struct
import random

CMD_GET_STATUS = struct.pack('q',0x0902)

class Edentifier(object):

	def __init__(self):
		self.edf = usb.core.find(idVendor=0x0b0c,idProduct=0x001e)

		# Listen for e-dentifier 2
		print("Waiting for e-dentifier 2...")
		while self.edf == None:
			self.edf = usb.core.find(idVendor=0x0b0c,idProduct=0x001e)

		random.seed()

	def setup(self):
		print("Setting up e-dentifier 2...")
		self.edf.set_configuration()

	def write(self, data=''):
		cfg = self.edf.get_active_configuration()
		intf = cfg[(0,0)]
		ep = usb.util.find_descriptor(intf, custom_match=\
		lambda e: \
			usb.util.endpoint_direction(e.bEndpointAddress) == \
			usb.util.ENDPOINT_OUT)
		assert ep is not None
		return ep.write(data)

	def read(self):
		cfg = self.edf.get_active_configuration()
		intf = cfg[(0,0)]
		ep = usb.util.find_descriptor(intf, custom_match=\
		lambda e: \
			usb.util.endpoint_direction(e.bEndpointAddress) == \
			usb.util.ENDPOINT_OUT)
		assert ep is not None
		return ep.read(8)

	def GetCardStatus(self):
		print("Retrieving status...")
		return self.write(CMD_GET_STATUS)

if __name__ == "__main__":
	target = Edentifier()
	target.setup()

	# Weird bug? Initialiation issue in USB stack?
	ret = target.GetCardStatus()
	if ret != 0:
		print("GetCardStatus(): %d" % ret)
	else:
		print("Failed")
