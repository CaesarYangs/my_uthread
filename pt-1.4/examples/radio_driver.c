/*
A radio driver written both with protothreads and events
This example shows an interrupt handler in a device driver for a TR1001 radio
chip. The driver receives incoming data in bytes and constructs a frame that is
covered by a CRC checksum. The driver is implemented both with protothreads and
with an explicit state machine. The state machine has 11 states and is
implemented using the C switch() statement. In contrast, the protothreads-based
implementation does not have any explicit states.

The flow of control in the state machine-based implementation is quite hard to
follow from inspection of the code, whereas the flow of control is evident in
the protothreads based implementation.
*/

PT_THREAD(tr1001_rxhandler(unsigned char incoming_byte)) {
    PT_YIELDING();
    static unsigned char rxtmp, tmppos;

    PT_BEGIN(&rxhandler_pt);

    while (1) {
        /* Wait until we receive the first syncronization byte. */
        PT_WAIT_UNTIL(&rxhandler_pt, incoming_byte == SYNCH1);

        tr1001_rxstate = RXSTATE_RECEVING;

        /* Read all incoming syncronization bytes. */
        PT_WAIT_WHILE(&rxhandler_pt, incoming_byte == SYNCH1);

        /* We should receive the second synch byte by now, otherwise we'll
           restart the protothread. */
        if (incoming_byte != SYNCH2) {
            PT_RESTART(&rxhandler_pt);
        }

        /* Reset the CRC. */
        rxcrc = 0xffff;

        /* Read packet header. */
        for (tmppos = 0; tmppos < TR1001_HDRLEN; ++tmppos) {
            /* Wait for the first byte of the packet to arrive. */
            PT_YIELD(&rxhandler_pt);

            /* If the incoming byte isn't a valid Manchester encoded byte,
               we start again from the beinning. */
            if (!me_valid(incoming_byte)) {
                PT_RESTART(&rxhandler_pt);
            }

            rxtmp = me_decode8(incoming_byte);

            /* Wait for the next byte to arrive. */
            PT_YIELD(&rxhandler_pt);

            if (!me_valid(incoming_byte)) {
                PT_RESTART(&rxhandler_pt);
            }

            /* Put together the two bytes into a single Manchester decoded
               byte. */

            tr1001_rxbuf[tmppos] = (rxtmp << 4) | me_decode8(incoming_byte);

            /* Calculate the CRC. */
            rxcrc = crc16_add(tr1001_rxbuf[tmppos], rxcrc);
        }

        /* Since we've got the header, we can grab the length from it. */
        tr1001_rxlen = ((((struct tr1001_hdr *)tr1001_rxbuf)->len[0] << 8) +
                        ((struct tr1001_hdr *)tr1001_rxbuf)->len[1]);

        /* If the length is longer than we can handle, we'll start from
           the beginning. */
        if (tmppos + tr1001_rxlen > sizeof(tr1001_rxbuf)) {
            PT_RESTART(&rxhandler_pt);
        }

        /* Read packet data. */
        for (tmppos = 6; tmppos < tr1001_rxlen + TR1001_HDRLEN; ++tmppos) {
            PT_YIELD(&rxhandler_pt);

            if (!me_valid(incoming_byte)) {
                PT_RESTART(&rxhandler_pt);
            }

            rxtmp = me_decode8(incoming_byte);

            PT_YIELD(&rxhandler_pt);

            if (!me_valid(incoming_byte)) {
                PT_RESTART(&rxhandler_pt);
            }

            tr1001_rxbuf[tmppos] = (rxtmp << 4) | me_decode8(incoming_byte);
            rxcrc = crc16_add(tr1001_rxbuf[tmppos], rxcrc);
        }

        /* Read the frame CRC. */
        for (tmppos = 0; tmppos < 4; ++tmppos) {
            PT_YIELD(&rxhandler_pt);

            if (!me_valid(incoming_byte)) {
                PT_RESTART(&rxhandler_pt);
            }

            rxcrctmp = (rxcrctmp << 4) | me_decode8(incoming_byte);
        }

        if (rxcrctmp == rxcrc) {
            /* A full packet has been received and the CRC checks out. We'll
               request the driver to take care of the incoming data. */

            tr1001_drv_request_poll();

            /* We'll set the receive state flag to signal that a full frame
               is present in the buffer, and we'll wait until the buffer has
               been taken care of. */
            tr1001_rxstate = RXSTATE_FULL;
            PT_WAIT_UNTIL(&rxhandler_pt, tr1001_rxstate != RXSTATE_FULL);
        }
    }
    PT_END(&rxhandler_pt);
}