// Author: Sumbul Aftab
// Binary Star Client implmentation using ZeroMQ Client Server Architecture
#include "czmq.h"
#define REQUEST_TIMEOUT     1000    //  in msecs
#define SETTLE_DELAY        2000    //  Time before failing over

// Main function
int main (void)
{
	// Initialize Context
    zctx_t *ctx = zctx_new ();
	
	// Primary server: 5001
	// Backup server: 5002
    char *server [] = { "tcp://localhost:5001", "tcp://localhost:5002" };
    uint server_nbr = 0;

	// Try connecting to server
    printf ("I: connecting to server at %s...\n", server [server_nbr]);
    void *client = zsocket_new (ctx, ZMQ_REQ);
    // Connect client at socket
    zsocket_connect (client, server [server_nbr]);
	
	// Request sequence generation
    int sequence = 0;
    while (!zctx_interrupted) {
        // Send a request
        char request [10];
        sprintf (request, "%d", ++sequence);
        zstr_send (client, request);
		
		// Polling socket for a reply
		// Expect timeout
        int expect_reply = 1;
        while (expect_reply) {
            zmq_pollitem_t items [] = { { client, 0, ZMQ_POLLIN, 0 } };
            int rc = zmq_poll (items, 1, REQUEST_TIMEOUT * ZMQ_POLL_MSEC);
            if (rc == -1)
                break;          //  If thread is interrupted

            
            // Client Startegy: Lazy pirate
            // No response during timeut period = Close socket and try again
            // Binary Star implementation:
            // Client vote decides whether server is priamry or backup
            // Client connects to each server in turns
            
            // Reply received from server
            if (items [0].revents & ZMQ_POLLIN) {
                //  Reply matches sequence?
                char *reply = zstr_recv (client);
                if (atoi (reply) == sequence) {
                	// Sequence matched
                    printf ("I: server replied OK (%s)\n", reply);
                    expect_reply = 0;
                    sleep (1);  //  One request per second
                }
                else
                	// Sequence not matched
                    printf ("E: bad reply from server: %s\n", reply);
                free (reply);
            }
            // No response received from server
            else {
                printf ("W: no response from server, failing over\n");
                
                // Close current socket and connect to other socket
                zsocket_destroy (ctx, client);
                server_nbr = (server_nbr + 1) % 2;
                zclock_sleep (SETTLE_DELAY);
                // Connect to other server
                printf ("I: connecting to server at %s...\n",
                        server [server_nbr]);
                client = zsocket_new (ctx, ZMQ_REQ);
                zsocket_connect (client, server [server_nbr]);

                //  Send request again, on new socket
                zstr_send (client, request);
            }
        }
    }
    // Destroy Context
    zctx_destroy (&ctx);
    return 0;
}
