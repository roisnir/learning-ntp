#ifndef NTP_PACKET
#define NTP_PACKET
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

#endif /* NTP_PACKET */