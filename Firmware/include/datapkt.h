# define PACKET_MAX_DATA 500

typedef struct datapkt_s {
    uint8_t magic;
    uint16_t pktsize;
    uint16_t data_size;
    uint16_t id;
    uint16_t total; // total num of packets
    // uint8_t data[PACKET_MAX_DATA];
    // uint8_t (*pktdata)[PACKET_MAX_DATA];
    uint8_t pktdata[PACKET_MAX_DATA];

    datapkt_s() : magic(0xAB) {}
} datapkt_t;

