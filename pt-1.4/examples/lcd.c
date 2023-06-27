// Delays - text scrolling on an LCD panel
// Protothreads can be used for introducing delays inside a function, without
// using a full threading model. The following example shows a function writing
// text to a one-line LCD panel. If the text is longer than the size of the
// panel, the text should be scrolling in from the right.

#include "../pt.h"
#include "timer.h"

struct state {
    char *text;
    char *scrollptr;
    struct pt pt;
    struct timer timer;
};

PT_THREAD(display_text(struct state *s)) {
    PT_BEGIN(&s->pt);

    /* If the text is shorter than the display size, show it right
       away. */
    if (strlen(s->text) <= LCD_SIZE) {
        lcd_display_text(s->text);
    } else {
        /* If the text is longer than the display, we should scroll in the
           text from the right with a delay of one second per scroll
           step. We do this in a for() loop, where the loop variable is
           the pointer to the first character to be displayed. */
        for (s->scrollptr = s->text; strlen(s->scrollptr) > LCD_SIZE;
             ++s->scrollptr) {
            lcd_display_text(s->scrollptr);

            /* Wait for one second. */
            timer_set(&s->timer, ONE_SECOND);
            PT_WAIT_UNTIL(&s->pt, timer_expired(&s->timer));
        }
    }

    PT_END(&s->pt);
}