import os.path
import re
import logging
import sys

import pcbnew as p





class StreamToLogger(object):
   """
   Fake file-like stream object that redirects writes to a logger instance.
   """
   def __init__(self, logger, log_level=logging.INFO):
      self.logger = logger
      self.log_level = log_level
      self.linebuf = ''

   def write(self, buf):
      for line in buf.rstrip().splitlines():
         self.logger.log(self.log_level, line.rstrip())

class RoundyTraces(p.ActionPlugin):
    """
    This will roundify your traces.
    """
    def setup_log(self):
        log_file = r'C:\Users\kangasp\Documents\KiCad\6.0\scripting\plugins\plugin.log'
        # log_file = os.path.join(b.GetFileName(), "component_layout_plugin.log")
        filehandler = logging.FileHandler(log_file)
        filehandler.setLevel(logging.DEBUG)
        formatter = logging.Formatter('%(asctime)s %(name)s %(lineno)d:%(message)s')
        filehandler.setFormatter(formatter)
        logger = logging.getLogger(__name__)
        # The log setup is persistent accross plugin runs because the kicad python 
        # kernel keeps running, so clear any existing handlers to avoid multiple 
        # outputs
        while len(logger.handlers) > 0:
            logger.removeHandler(logger.handlers[0])
        logger.addHandler(filehandler)
        logger.setLevel(logging.DEBUG)
        self.log = logger
        # Redirect stdout and stderr to logfile
        stdout_logger = logging.getLogger('STDOUT')
        sl_out = StreamToLogger(stdout_logger, logging.INFO)
        sys.stdout = sl_out
        stderr_logger = logging.getLogger('STDERR')
        sl_err = StreamToLogger(stderr_logger, logging.ERROR)
        sys.stderr = sl_err
        self.log.info('Logging to {0}'.format(log_file))
        self.log.info("Executing {0}".format(__file__))



    def defaults(self):
        self.name = "Roundify Traces"
        self.category = "Modify Traces"
        self.description = "Update the traces that can be, to be circles/arcs."
        self.show_toolbar_button = True

    def Run(self):
        """
        Do the actual rounding.
        """
        self.setup_log()
        b = p.GetBoard()
        netcodes = b.GetNetsByNetcode()
        allTracks = []
        for netcode, net in netcodes.items():
            self.log.info("netcode: {0}, net: {1}, netclass: {2}".format(netcode, net.GetNetname(), net.GetNetClassName() ))
            trks = b.TracksInNet(netcode) # get all the tracks in this net
            def share_point(t1,t2):
                if( not t1.IsPointOnEnds(t2.GetStart)) and 
                        ( not trks[t2].IsPointOnEnds(trks[t1].GetEnd())):
            starts = set() 
            ends = set() 
            for t1 in trks:
                starts.add(t1.GetStart())
                ends.add(t1.GetEnd())
                sx, sy = t1.GetStart()
                ex, ey = t1.GetEnd()
                self.log.info("trks: start: [{0},{1}], end: [{2},{3}], len: {4}".format(sx,sy,ex,ey, t1.GetLength()))
            shared = starts.union(ends)
            sq = starts.difference(shared)
            se = ends.difference(shared)


        print("helloWorld")

RoundyTraces().register()

'''
    (sp, ep, width, layer, net) = trackpoints

    track = pcbnew.TRACK(board)
    track.SetStart(sp)
    track.SetEnd(ep)
    track.SetWidth(width)
    track.SetLayer(layer)
    board.Add(track)
    track.SetNet(net)

'''
