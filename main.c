#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

uint8_t IN_BUF[1024 * 1024] = {0};
size_t MAX_PACKET_SIZE = 1024;
size_t MAX_PACKETS_IN_FLIGHT = 1024;

typedef struct packet {
    uint8_t *data;
    size_t size;
} packet;

int main() {
    printf("Size of IN_BUF: %zu MB\n", sizeof(IN_BUF) / 1024 / 1024);
    uint8_t *p = IN_BUF;
    uint8_t *c = IN_BUF;

    // packet format = 0xAB, [uint32_t len], [payload], 0xBA, 0xBA
    uint8_t data_arr[10] = {0xAB, 10, 0, 0, 0, 5, 6, 7, 0xBA, 0xBA};
    packet p1 = {
        .data = data_arr,
        .size = 10
    };

    memcpy(p, p1.data, p1.size);
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