#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

uint8_t IN_BUF[1024 * 1024] = {0};
size_t MAX_PACKET_SIZE = 1024;
size_t MAX_PACKETS_IN_FLIGHT = 1024;

// start BABECAFE, end C0FFEE
// len = payload_len + 15 (overhead)
size_t PACKET_OVERHEAD = 15;
// packet format = 0xBA, 0xBE, 0xCA, 0xFE, [uint32_t len], [payload], [CRC32 (0's if unused)], 0xC0, 0xFF, 0xEE

typedef struct packet {
    uint8_t *data;
    size_t len;
} packet;

void insert_dummy_packet(uint8_t **p, size_t len) {
    // bounds checking first
    if (len < PACKET_OVERHEAD) { printf("Packet cannot be shorter than the overhead.\n"); return; }
    if (!*p) { printf("Unable to write into the producer queue, p is null.\n"); return; }
    bool needs_wraparound = false;
    if (*p + len > IN_BUF + MAX_PACKET_SIZE * MAX_PACKETS_IN_FLIGHT) needs_wraparound = true;
    uint8_t start_delim[4] = {0xBA, 0xBE, 0xCA, 0xFE};
    uint8_t end_delim[3] = {0xC0, 0xFF, 0xEE};
    uint8_t blank_crc[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t len_uint8[4];
    uint8_t *len_addr = (uint8_t *) &len;
    for (size_t i = 0; i < 4; i++) {
        len_uint8[i] = len_addr[i];
    }

    switch (needs_wraparound) {
        case false:
            memcpy(*p, start_delim, 4); *p += 4;
            memcpy(*p, len_uint8, 4); *p += 4;
            for (size_t i = 0; i < len - PACKET_OVERHEAD; i++) {
                *(*p)++ = i;
            }
            memcpy(*p, blank_crc, 4); *p += 4;
            memcpy(*p, end_delim, 3); *p += 3;
            break;
        case true:
            uint8_t staging_buffer[MAX_PACKET_SIZE];
            uint8_t *stage_p = staging_buffer;
            size_t space_available = IN_BUF + (MAX_PACKET_SIZE * MAX_PACKETS_IN_FLIGHT) - *p;
            memcpy(staging_buffer, start_delim, 4); stage_p += 4;
            memcpy(stage_p, len_uint8, 4); stage_p += 4;
            for (size_t i = 0; i < len - PACKET_OVERHEAD; i++) {
                *stage_p++ = i;
            }
            memcpy(stage_p, blank_crc, 4); stage_p += 4;
            memcpy(stage_p, end_delim, 3); stage_p += 3;

            memcpy(*p, staging_buffer, space_available); *p = IN_BUF;
            stage_p = staging_buffer + space_available;
            memcpy(*p, stage_p, len - space_available); *p += len - space_available;
            break;
    }
    return;    
}

int main() {
    printf("Size of IN_BUF: %zu MB\n", sizeof(IN_BUF) / 1024 / 1024);
    uint8_t *p = IN_BUF;
    uint8_t *c = IN_BUF;

    uint8_t data_arr[10] = {0xAB, 10, 0, 0, 0, 5, 6, 7, 0xBA, 0xBA};
    packet p1 = {
        .data = data_arr,
        .len = 10
    };

    memcpy(p, p1.data, p1.len);
    p += MAX_PACKET_SIZE;

    if (c != p) {
        uint8_t *tmp = c;

        // sanity check
        if (*tmp++ != 0xAB) { printf ("Packet is corrupt.\n"); return 1; }

        // first get the length of the packet
        size_t len = 0;
        for (size_t i = 0; i < 4; i++) {
            printf("i = %zu, *tmp = %d\n", i, *tmp);
            len += ((size_t) *tmp++) << 8*i;
        }
        printf("Length of packet: %lu\n", len);
        printf("p: %p, c: %p, packets in flight: %zu\n", p, c, (p - c) / MAX_PACKET_SIZE);

        // sanity check
        printf("*(tmp + len - 7) = %d, *(tmp + len - 6) = %d\n", *(tmp + len - 7), *(tmp + len - 6));
        if (*(tmp + len - 7) != 0xBA || *(tmp + len - 6) != 0xBA) { printf ( "Packet is corrupt.\n"); }

        uint8_t *payload = malloc(len - 7);
        memcpy(payload, tmp, len - 7);
        c += MAX_PACKET_SIZE;

        printf("Payload received: ");
        for (size_t i = 0; i < len - 7; i++) {
            printf("%d ", *(payload + i));
        }
        printf("\n");

        free(payload);
    }

    printf("p: %p, c: %p, packets in flight: %zu\n", p, c, (p - c) / MAX_PACKET_SIZE);
    return 0;
}