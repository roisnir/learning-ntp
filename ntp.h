#ifndef NTP
#define NTP

#define NTP_PORT 123
#define NTP_MODE(b) (b & 0x7)
#define NTP_VERSION(b) ((b >> 3) & 0x7)
#define NTP_LEAP_INDICATOR(b) (b >> 6)
#define NTP_TIMESTAMP_DELTA 2208988800ul
#include <stdint.h>

typedef struct {
  uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                           // li.   Two bits.   Leap indicator.
                           // vn.   Three bits. Version number of the protocol.
                           // mode. Three bits. Client will pick mode 3 for client.

  uint8_t stratum;         // Unsigned Eight bits. Stratum level of the local clock.
  int8_t poll;            // Signed Eight bits. log2 sec. Maximum interval between successive messages.
  int8_t precision;       // Signed Eight bits. log2 sec. Precision of the local clock.

  uint16_t rootDelay_s;      // 16 bits. Total round trip delay time secs.
  uint16_t rootDelay_f;      // 16 bits. Total round trip delay time fraction.
  uint16_t rootDispersion_s; // 32 bits. Max error aloud from primary clock source secs.
  uint16_t rootDispersion_f; // 32 bits. Max error aloud from primary clock source fraction.
  uint32_t refId;          // 32 bits. Reference clock identifier.

  uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
  uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

  uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
  uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

  uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
  uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

  uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
  uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet;              // Total: 384 bits or 48 bytes.

/*
Converts an `ntp_packet`'s byte order from network to host.
packet: pointer to an ntp packet
*/
void ntoh_ntp_packet(ntp_packet *packet);

/*
Converts an `ntp_packet`'s byte order from host to network.
packet: pointer to an ntp packet
*/
void hton_ntp_packet(ntp_packet *packet);

/*
Converts an ntp timestamp to epoch timestamp
ntp_ts: seconds past since 1900-1-1 UTC
returns: seconds past since 1970-1-1 UTC
*/
uint32_t ntp_ts_to_epoch(uint32_t ntp_ts);

void print_ntp_packet(ntp_packet *pkt);

#endif /* NTP */