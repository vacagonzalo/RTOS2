# Copyright 2021 Lucas Orsi (lorsi)
#                Franco Bucafusco
# Redistribution and use in source and binary forms, with or without modification, 
# are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this 
#    list of conditions and the following disclaimer.
# 
# 2. Redistributions in binary form must reproduce the above copyright notice, 
#    this list of conditions and the following disclaimer in the documentation 
#    and/or other materials provided with the distribution.
# 
# 3. Neither the name of the copyright holder nor the names of its contributors 
#    may be used to endorse or promote products derived from this software without 
#    specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
# IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
# OF SUCH DAMAGE.
# 
import unittest
import serial
import time
from serial_init import *

def send_receive( port , tosend): 
    port.write( tosend.encode() )
    print("Sent: " + tosend)
    r = port.readall().decode('utf-8')  
    print("Received: " + r)
    return r
 
class unit_tests(unittest.TestCase):

    def setUp(self) -> None:
        
        #seleccion de puerto serie
        puerto = serial_choose()
        # print("Puerto serie elegido : " + puerto)

        #declaro el objeto de puerto serie (sin abrirlo)
        self.Port = serial.Serial()
        
        #parametros del puerto serie
        self.Port.port       = puerto
        self.Port.baudrate   = 115200
        self.Port.bytesize   = serial.EIGHTBITS      # number of bits per bytes # SEVENBITS
        self.Port.parity     = serial.PARITY_NONE    # set parity check: no parity # PARITY_ODD
        self.Port.stopbits   = serial.STOPBITS_ONE   # number of stop bits # STOPBITS_TWO
        self.Port.timeout    = .1                     # non-block read

        try:

            self.Port.open()
            self.Port.flushInput()       # flush input buffer, discarding all its contents
            self.Port.flushOutput()      # flush output buffer, aborting current output
                            # and discard all that is in buffer
            
        except Exception as e:
            print("Error abriendo puerto serie.\n" + str(e) + '\nFin de programa.')
            exit()

    def tearDown(self) -> None:
        # print("closing port")
        self.Port.close()

    def test_echo(self):
        # print("executing echo test")
        tstmsg = '(0000CHelloE9)'
        expmsg = '(0000Chello8D)'
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )
    
    def test_pascal_camel(self):
        # print("executing pascal_camel test")
        tstmsg = '(0000CHelloWorld97)'
        expmsg = '(0000ChelloWorldB4)'
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )
    
    def test_camel_snake(self):
        # print("executing camel_snake test")
        tstmsg = '(0000ShelloWorld43)'
        expmsg = '(0000Shello_world2E)'
        self.Port.write(tstmsg.encode('utf-8'))
        r = self.Port.readall() .decode('utf-8')
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )
    
    def test_pascal_snake(self):
        # print("executing pascal_snake test")
        tstmsg = '(0000SHelloWorld60)'
        expmsg = '(0000Shello_world2E)'
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )
    
    def test_camel_pascal(self):
        # print("executing camel_pascal test")
        tstmsg = '(0000PfooBarBazF2)'
        expmsg = '(0000PFooBarBazFF)'
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )

    def test_snake_camel(self):
        # print("executing snake_camel test")
        tstmsg = '(0000Chello_worldE5)'
        expmsg = '(0000ChelloWorldB4)'
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )
    
    def test_multi(self):
        # print("executing multi test")
        tstmsg = '(0000CHelloE9)'
        expmsg = '(0000Chello8D)'
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )

        tstmsg = '(0000SGoodbye89)'
        expmsg = '(0000Sgoodbye28)'
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )

        tstmsg = '(0000CHelloE9)'
        expmsg = '(0000Chello8D)'
        self.assertEqual( expmsg , send_receive(self.Port , tstmsg) )
    
    def test_crc_ignore(self):
        # print("executing crc_ignore test")
        tstmsg = '(0000CHelloE9)'
        expmsg = '(0000Chello8D)'
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )

        tstmsg = '(0000SGoodbye87)'
        expmsg = ''
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )

        tstmsg = '(0000CHelloE9)'
        expmsg = '(0000Chello8D)'
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )
    
    def test_rx_timeout(self):
        # print("executing rx_timeout test")
        tstmsg = '(0000SGoodbye89)'
        expmsg = '(0000Sgoodbye28)'
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )
        
        # Should be discarded.
        tstmsg = '(0000CHelloE9'
        expmsg = ''
        send_receive( self.Port , tstmsg)  

        tstmsg = '(0000SGoodbye89)'
        expmsg = '(0000Sgoodbye28)'
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )
    
    def test_sends_error_on_invalid_oppcode(self):
        # print("executing sends_error_on_invalid_oppcode test")
        tstmsg = '(0000QGoodbyeAF)'
        expmsg = '(0000E0100)'
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )

    #ctivar este test en caso de requerirlo

    # def test_sends_error_on_excesive_words(self):
    #     print("executing sends_error_on_excesive_words test")
    #     tstmsg = '(0000COneTwoThreeFourrFiveSixSevenEightNineTenElvenTwelveThirteenFourteenFifteenSixteenSeventeenEighteenF8)'
    #     expmsg = '(0000E0007)'
    #     self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )

    #     tstmsg = '(0000Cone_two_three_four_five_one_two_three_four_five_one_two_three_four_five_one_two_three_four_fiveone_two_three_four_fiveBA)'
    #     expmsg = '(0000E0007)'
    #     self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )
    
    def test_ignores_zero_words(self):
        # print("executing ignores_zero_words test")
        tstmsg = '(000080)'
        expmsg = ''
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )
    
    def test_ignores_empty_packet(self):
        # print("executing ignores_empty_packet test")
        tstmsg = ''
        expmsg = ''
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )
    
    def test_fails_empty_message(self):
        # print("executing fails_empty_message test")
        tstmsg = '(000080)'
        expmsg = ''
        self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )

    #test_sends_error_on_excesive_chars

    # def test_sends_error_on_excesive_chars(self):
    #     # print("executing sends_error_on_excesive_chars test")
    #     tstmsg = '(0000CSuperlongwordthatnoonecaresabout5D)'
    #     expmsg = '(0000E0007)'
    #     self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )

    #ctivar este test en caso de requerirlo
    
    # def test_sends_error_on_special_char_at_end(self):
    #     print("executing test_sends_error_on_special_char_at_end")
    #     tstmsg = '(0000Shello_world_50)'
    #     expmsg = '(0000E0007)'
    #     self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )

    #     tstmsg = '(0000Shello__world11)'
    #     expmsg = '(0000E0007)'
    #     self.assertEqual( expmsg , send_receive( self.Port , tstmsg) )

if __name__ == '__main__':
    unittest.main()