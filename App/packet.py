from struct import Struct

class Datapacket:
    def __init__(self):
        self.fmt = Struct('<B4H500s')
        self.magic = None
        self.pktsize = None
        self.datasize = None
        self.id = None
        self.totalpkts = None
        self.data = None
        

    def from_bin(self, bin):
        res = self.fmt.unpack(bin)
        self.magic = res[0]
        self.pktsize = res[1]
        self.datasize = res[2]
        self.id = res[3]
        self.totalpkts = res[4]
        self.data = res[5]


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
    
    

