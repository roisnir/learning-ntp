#include "ntp.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>

/*
Converts an `ntp_packet`'s byte order from network to host.
packet: pointer to an ntp packet
*/
void ntoh_ntp_packet(ntp_packet *packet) {
    packet->rootDelay_s = ntohs(packet->rootDelay_s);
    packet->rootDelay_f = ntohs(packet->rootDelay_f);
    packet->rootDispersion_s = ntohs(packet->rootDispersion_s);
    packet->rootDispersion_f = ntohs(packet->rootDispersion_f);
    packet->refTm_s = ntohl(packet->refTm_s);
    packet->refTm_f = ntohl(packet->refTm_f);
    packet->origTm_s = ntohl(packet->origTm_s);
    packet->origTm_f = ntohl(packet->origTm_f);
    packet->rxTm_s = ntohl(packet->rxTm_s);
    packet->rxTm_f = ntohl(packet->rxTm_f);
    packet->txTm_s = ntohl(packet->txTm_s);
    packet->txTm_f = ntohl(packet->txTm_f);
}

/*
Converts an `ntp_packet`'s byte order from host to network.
packet: pointer to an ntp packet
*/
void hton_ntp_packet(ntp_packet *packet) {
    packet->rootDelay_s = htons(packet->rootDelay_s);
    packet->rootDelay_f = htons(packet->rootDelay_f);
    packet->rootDispersion_s = htons(packet->rootDispersion_s);
    packet->rootDispersion_f = htons(packet->rootDispersion_f);
    packet->refTm_s = htonl(packet->refTm_s);
    packet->refTm_f = htonl(packet->refTm_f);
    packet->origTm_s = htonl(packet->origTm_s);
    packet->origTm_f = htonl(packet->origTm_f);
    packet->rxTm_s = htonl(packet->rxTm_s);
    packet->rxTm_f = htonl(packet->rxTm_f);
    packet->txTm_s = htonl(packet->txTm_s);
    packet->txTm_f = htonl(packet->txTm_f);
}

/*
Converts an ntp timestamp to epoch timestamp
ntp_ts: seconds past since 1900-1-1 UTC
returns: seconds past since 1970-1-1 UTC
*/
uint32_t ntp_ts_to_epoch(uint32_t ntp_ts) {
    if (ntp_ts == 0) {
        return 0;
    }
    return ntp_ts - NTP_TIMESTAMP_DELTA;
}


void print_ntp_packet(ntp_packet *pkt) {
    char refts[35];
    char orgts[35];
    char rects[35];
    char trats[35];
    isoformatts(refts, ntp_ts_to_epoch(pkt->refTm_s), pkt->refTm_f);
    isoformatts(orgts, ntp_ts_to_epoch(pkt->origTm_s), pkt->origTm_f);
    isoformatts(rects, ntp_ts_to_epoch(pkt->rxTm_s), pkt->rxTm_f);
    isoformatts(trats, ntp_ts_to_epoch(pkt->txTm_s), pkt->txTm_f);
    char *ref_id;
    if (pkt->stratum == 1) {
        ref_id = (char *)&pkt->refId;
    } else {
        struct in_addr tmp;
        tmp.s_addr = pkt->refId;
        ref_id = inet_ntoa(tmp);
    }
    printf(
        "NTP packet:\n"
        "  Flags:\n"
        "    Leap Indicator: %d\n"
        "    Version: %d\n"
        "    Mode: %d\n"
        "  Peer Clock Stratum: %d\n"
        "  Peer Polling Interval: %f\n"
        "  Peer Clock Precision: %f\n"
        "  Root Delay: %f\n"
        "  Root Dispersion: %f\n"
        "  Reference ID: %s\n"
        "  Reference Timestamp: %s\n"
        "  Origin Timestamp: %s\n"
        "  Receive Timestamp: %s\n"
        "  Transmit Timestamp: %s\n",
        NTP_LEAP_INDICATOR(pkt->li_vn_mode), NTP_VERSION(pkt->li_vn_mode),
        NTP_MODE(pkt->li_vn_mode), pkt->stratum, pow(2, (double)pkt->poll),
        pow(2, (double)pkt->precision),
        pkt->rootDelay_s + (((double)pkt->rootDelay_f) / 0xFFFF),
        pkt->rootDispersion_s + ((((double)pkt->rootDispersion_f)) / 0xFFFF),
        ref_id, refts, orgts, rects, trats);
}