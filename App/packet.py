from struct import Struct, unpack
from enum import IntEnum

class Datapacket:
    def __init__(self):
        self.fmt = Struct('<I4H')
        self.magic = None
        self.pktsize = None
        self.datasize = None
        self.id = None
        self.totalpkts = None
        self.data = None
        

    def from_bin(self, bin):

        res = self.fmt.unpack(bin[0:12])
        self.magic = res[0]
        self.pktsize = res[1]
        self.datasize = res[2]
        self.id = res[3]
        self.totalpkts = res[4]

        self.data = bin[12:]


class Ackpacket:
    def __init__(self) -> None:
        self.fmt = Struct('<H')
        self.id = None
    
    def __init__(self, id) -> None:
        self.fmt = Struct('<H')
        self.id = id

    def set_id(self, id):
        self.id = id
    
    def get_bin(self):
        return self.fmt.pack(self.id)
    

class Cmd(IntEnum):
    SETINTERVAL = 1
    POWEROFF = 2

class Cmdpacket:
    def __init__(self) -> None:
        self.fmt = Struct('<II')
        self.cmd = None
        self.par = None

    def __init__(self, cmd, par) -> None:
        self.fmt = Struct('<II')
        self.cmd = cmd
        self.par = par
    
    def from_bin(self, bin):
        self.cmd, self.par = self.fmt.unpack(bin)
    
    def get_bin(self):
        return self.fmt.pack(int(self.cmd), self.par)


