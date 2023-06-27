/*A code lock
This example is a bit more complicated and shows how to implement a simple code
lock - the kind of device that is placed next to doors and that you have to push
a four digit number into in order to unlock the door.

The code lock waits for key presses from a numeric keyboard and if the correct
code is entered, the lock is unlocked. There is a maximum time of one second
between each key press, and after the correct code has been entered, no more
keys must be pressed for 0.5 seconds before the lock is opened.
*/

/*
 * This is the code that has to be entered.
 */
static const char code[4] = {'1', '4', '2', '3'};

/*
 * Declaration of the protothread function implementing the code lock
 * logic. The protothread function is declared using the PT_THREAD()
 * macro. The function is declared with the "static" keyword since it
 * is local to this file. The name of the function is codelock_thread
 * and it takes one argument, pt, of the type struct pt.
 *
 */
static PT_THREAD(codelock_thread(struct pt *pt)) {
    /* This is a local variable that holds the number of keys that have
     * been pressed. Note that it is declared with the "static" keyword
     * to make sure that the variable is *not* allocated on the stack.
     */
    static int keys;

    /*
     * Declare the beginning of the protothread.
     */
    PT_BEGIN(pt);

    /*
     * We'll let the protothread loop until the protothread is
     * expliticly exited with PT_EXIT().
     */
    while (1) {
        /*
         * We'll be reading key presses until we get the right amount of
         * correct keys.
         */
        for (keys = 0; keys < sizeof(code); ++keys) {
            /*
             * If we haven't gotten any keypresses, we'll simply wait for one.
             */
            if (keys == 0) {
                /*
                 * The PT_WAIT_UNTIL() function will block until the condition
                 * key_pressed() is true.
                 */
                PT_WAIT_UNTIL(pt, key_pressed());
            } else {
                /*
                 * If the "key" variable was larger than zero, we have already
                 * gotten at least one correct key press. If so, we'll not
                 * only wait for the next key, but we'll also set a timer that
                 * expires in one second. This gives the person pressing the
                 * keys one second to press the next key in the code.
                 */
                timer_set(&codelock_timer, 1000);

                /*
                 * The following statement shows how complex blocking
                 * conditions can be easily expressed with protothreads and
                 * the PT_WAIT_UNTIL() function.
                 */
                PT_WAIT_UNTIL(pt,
                              key_pressed() || timer_expired(&codelock_timer));

                /*
                 * If the timer expired, we should break out of the for() loop
                 * and start reading keys from the beginning of the while(1)
                 * loop instead.
                 */
                if (timer_expired(&codelock_timer)) {
                    printf("Code lock timer expired.\n");

                    /*
                     * Break out from the for() loop and start from the
                     * beginning of the while(1) loop.
                     */
                    break;
                }
            }

            /*
             * Check if the pressed key was correct.
             */
            if (key != code[keys]) {
                printf("Incorrect key '%c' found\n", key);
                /*
                 * Break out of the for() loop since the key was incorrect.
                 */
                break;
            } else {
                printf("Correct key '%c' found\n", key);
            }
        }

        /*
         * Check if we have gotten all keys.
         */
        if (keys == sizeof(code)) {
            printf(
                "Correct code entered, waiting for 500 ms before unlocking.\n");

            /*
             * Ok, we got the correct code. But to make sure that the code
             * was not just a fluke of luck by an intruder, but the correct
             * code entered by a person that knows the correct code, we'll
             * wait for half a second before opening the lock. If another
             * key is pressed during this time, we'll assume that it was a
             * fluke of luck that the correct code was entered the first
             * time.
             */
            timer_set(&codelock_timer, 500);
            PT_WAIT_UNTIL(pt, key_pressed() || timer_expired(&codelock_timer));

            /*
             * If we continued from the PT_WAIT_UNTIL() statement without
             * the timer expired, we don't open the lock.
             */
            if (!timer_expired(&codelock_timer)) {
                printf(
                    "Key pressed during final wait, code lock locked again.\n");
            } else {
                /*
                 * If the timer expired, we'll open the lock and exit from the
                 * protothread.
                 */
                printf("Code lock unlocked.\n");
                PT_EXIT(pt);
            }
        }
    }

    /*
     * Finally, we'll mark the end of the protothread.
     */
    PT_END(pt);
}