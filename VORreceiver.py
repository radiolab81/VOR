#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Vorreceiver
# Generated: Mon Mar 29 14:04:40 2021
##################################################


from gnuradio import analog
from gnuradio import audio
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import fft
from gnuradio import filter
from gnuradio import gr
from gnuradio import iio
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from math import pi
from optparse import OptionParser
import SimpleXMLRPCServer
import osmosdr
import threading
import time


class VORreceiver(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "Vorreceiver")

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 2048000
        self.freq = freq = 110450000

        ##################################################
        # Blocks
        ##################################################
        self.xmlrpc_server_0 = SimpleXMLRPCServer.SimpleXMLRPCServer(('127.0.0.1', 7777), allow_none=True)
        self.xmlrpc_server_0.register_instance(self)
        self.xmlrpc_server_0_thread = threading.Thread(target=self.xmlrpc_server_0.serve_forever)
        self.xmlrpc_server_0_thread.daemon = True
        self.xmlrpc_server_0_thread.start()
        self.rtlsdr_source_0 = osmosdr.source( args="numchan=" + str(1) + " " + '' )
        self.rtlsdr_source_0.set_sample_rate(samp_rate)
        self.rtlsdr_source_0.set_center_freq(freq, 0)
        self.rtlsdr_source_0.set_freq_corr(0, 0)
        self.rtlsdr_source_0.set_dc_offset_mode(0, 0)
        self.rtlsdr_source_0.set_iq_balance_mode(0, 0)
        self.rtlsdr_source_0.set_gain_mode(True, 0)
        self.rtlsdr_source_0.set_gain(10, 0)
        self.rtlsdr_source_0.set_if_gain(20, 0)
        self.rtlsdr_source_0.set_bb_gain(20, 0)
        self.rtlsdr_source_0.set_antenna('', 0)
        self.rtlsdr_source_0.set_bandwidth(0, 0)

        self.rational_resampler_xxx_0 = filter.rational_resampler_ccc(
                interpolation=1,
                decimation=64,
                taps=None,
                fractional_bw=None,
        )
        self.low_pass_filter_0_1 = filter.fir_filter_ccf(1, firdes.low_pass(
        	1, samp_rate/64, 1000, 500, firdes.WIN_HAMMING, 6.76))
        self.low_pass_filter_0 = filter.fir_filter_fff(1, firdes.low_pass(
        	1, samp_rate/64, 1000, 500, firdes.WIN_HAMMING, 6.76))
        self.iio_modulo_const_ff_0 = iio.modulo_const_ff(360, 1)
        self.goertzel_fc_0_0 = fft.goertzel_fc(32000, 3200, 30)
        self.goertzel_fc_0 = fft.goertzel_fc(32000, 3200, 30)
        self.fir_filter_xxx_0 = filter.fir_filter_fff(1, (filter.optfir.low_pass(1, samp_rate/64, 100, 200, 0.1,  60)))
        self.fir_filter_xxx_0.declare_sample_delay(0)
        self.blocks_udp_sink_0 = blocks.udp_sink(gr.sizeof_float*1, '127.0.0.1', 1234, 1047, False)
        self.blocks_multiply_xx_0 = blocks.multiply_vcc(1)
        self.blocks_multiply_const_vxx_1 = blocks.multiply_const_vff((-180/pi, ))
        self.blocks_multiply_const_vxx_0 = blocks.multiply_const_vff((32, ))
        self.blocks_multiply_conjugate_cc_0 = blocks.multiply_conjugate_cc(1)
        self.blocks_float_to_complex_0 = blocks.float_to_complex(1)
        self.blocks_complex_to_arg_0 = blocks.complex_to_arg(1)
        self.blocks_add_const_vxx_1 = blocks.add_const_vff((360, ))
        self.band_pass_filter_0 = filter.fir_filter_fff(1, firdes.band_pass(
        	1, samp_rate/64, 920, 1120, 100, firdes.WIN_HAMMING, 6.76))
        self.audio_sink_0_0 = audio.sink(32000, '', False)
        self.analog_sig_source_x_0 = analog.sig_source_c(samp_rate/64, analog.GR_COS_WAVE, -9960, 1, 0)
        self.analog_fm_demod_cf_0 = analog.fm_demod_cf(
        	channel_rate=samp_rate/64,
        	audio_decim=1,
        	deviation=480*2,
        	audio_pass=100,
        	audio_stop=200,
        	gain=1.0,
        	tau=75e-6,
        )
        self.analog_am_demod_cf_0 = analog.am_demod_cf(
        	channel_rate=samp_rate/64,
        	audio_decim=1,
        	audio_pass=12000,
        	audio_stop=13000,
        )



        ##################################################
        # Connections
        ##################################################
        self.connect((self.analog_am_demod_cf_0, 0), (self.band_pass_filter_0, 0))
        self.connect((self.analog_am_demod_cf_0, 0), (self.blocks_float_to_complex_0, 0))
        self.connect((self.analog_am_demod_cf_0, 0), (self.low_pass_filter_0, 0))
        self.connect((self.analog_fm_demod_cf_0, 0), (self.goertzel_fc_0, 0))
        self.connect((self.analog_sig_source_x_0, 0), (self.blocks_multiply_xx_0, 1))
        self.connect((self.band_pass_filter_0, 0), (self.blocks_multiply_const_vxx_0, 0))
        self.connect((self.blocks_add_const_vxx_1, 0), (self.iio_modulo_const_ff_0, 0))
        self.connect((self.blocks_complex_to_arg_0, 0), (self.blocks_multiply_const_vxx_1, 0))
        self.connect((self.blocks_float_to_complex_0, 0), (self.blocks_multiply_xx_0, 0))
        self.connect((self.blocks_multiply_conjugate_cc_0, 0), (self.blocks_complex_to_arg_0, 0))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.audio_sink_0_0, 0))
        self.connect((self.blocks_multiply_const_vxx_1, 0), (self.blocks_add_const_vxx_1, 0))
        self.connect((self.blocks_multiply_xx_0, 0), (self.low_pass_filter_0_1, 0))
        self.connect((self.fir_filter_xxx_0, 0), (self.goertzel_fc_0_0, 0))
        self.connect((self.goertzel_fc_0, 0), (self.blocks_multiply_conjugate_cc_0, 1))
        self.connect((self.goertzel_fc_0_0, 0), (self.blocks_multiply_conjugate_cc_0, 0))
        self.connect((self.iio_modulo_const_ff_0, 0), (self.blocks_udp_sink_0, 0))
        self.connect((self.low_pass_filter_0, 0), (self.fir_filter_xxx_0, 0))
        self.connect((self.low_pass_filter_0_1, 0), (self.analog_fm_demod_cf_0, 0))
        self.connect((self.rational_resampler_xxx_0, 0), (self.analog_am_demod_cf_0, 0))
        self.connect((self.rtlsdr_source_0, 0), (self.rational_resampler_xxx_0, 0))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.rtlsdr_source_0.set_sample_rate(self.samp_rate)
        self.low_pass_filter_0_1.set_taps(firdes.low_pass(1, self.samp_rate/64, 1000, 500, firdes.WIN_HAMMING, 6.76))
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.samp_rate/64, 1000, 500, firdes.WIN_HAMMING, 6.76))
        self.fir_filter_xxx_0.set_taps((filter.optfir.low_pass(1, self.samp_rate/64, 100, 200, 0.1,  60)))
        self.band_pass_filter_0.set_taps(firdes.band_pass(1, self.samp_rate/64, 920, 1120, 100, firdes.WIN_HAMMING, 6.76))
        self.analog_sig_source_x_0.set_sampling_freq(self.samp_rate/64)

    def get_freq(self):
        return self.freq

    def set_freq(self, freq):
        self.freq = freq
        self.rtlsdr_source_0.set_center_freq(self.freq, 0)


def main(top_block_cls=VORreceiver, options=None):

    tb = top_block_cls()
    tb.start()
    tb.wait()


if __name__ == '__main__':
    main()
