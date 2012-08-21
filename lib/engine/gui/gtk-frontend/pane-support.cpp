/* added by jplace */

/* NOTE: I user stdout for errors here because Ekiga uses stderr for a lot of other things. It is easier debug from stderr. A change should eventually be made to using stderr */

#include <algorithm>
#include <iostream>
#include <vector>
#include <glib/gi18n.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>

#include "pane-support.h"

#define SERVERHOST "localhost"
#define SERVERPORT "4242"
#define SCHEDBUFSIZE 256
#define RESERVATION 1000000

char *user = "Jordan.";
char *reserve = "reserve(user=Jordan) = 1000000 on forJordan.";
char *getsched = "GetSchedule forJordan.";

/* returns socket fd or -1 on failure */
static int
connect_to_pane(char *host, char *port)
{
  int sockfd;
  struct addrinfo hints, *servinfo;
  int rv;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  servinfo = NULL;
  if(((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) || servinfo == NULL)
  {
	  fprintf(stdout, "Could not resolve address of PANE server.\n");
	  return -1;
  }

  if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
  {
	  fprintf(stdout, "Could not spawn socket for PANE connection.\n");
	  return -1;
  }

  if(connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
  {
	  close(sockfd);
	  fprintf(stdout, "Could not connect to PANE server,\n");
	  return -1;
  }

  freeaddrinfo(servinfo);

  return sockfd;
}


static int
query_user_call_length()
{
  /* ask how long call should be */
  GtkWidget *call_dialog = gtk_dialog_new_with_buttons("How long is your call?", NULL, GTK_DIALOG_MODAL, "Ok", GTK_RESPONSE_OK, NULL);
  gtk_widget_set_usize(call_dialog, 350, 200);
  
  /* set up dialog content */
  GtkWidget *call_content = gtk_dialog_get_content_area(GTK_DIALOG (call_dialog));
  GtkWidget *call_label = gtk_label_new("Your call quality cannot be guaranteed right now.\nHow many minutes do you anticipate this call being?");
  GtkWidget *call_entry = gtk_entry_new_with_max_length(5);
  gtk_container_add(GTK_CONTAINER (call_content), call_label);
  gtk_container_add(GTK_CONTAINER (call_content), call_entry);
 
  /* throw up the dialog */
  gtk_widget_show(call_label);
  gtk_widget_show(call_entry);
  gtk_widget_show(call_content);
  
  gtk_dialog_run(GTK_DIALOG (call_dialog));

  /* get call time */
  const gchar *call_buffer = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY (call_entry)));
  int time = atoi(call_buffer); /* TODO: some sort of error checking? */

  gtk_widget_destroy(call_dialog);

  return time;

}

static int
query_user_generic2(char *message)
{
  /* create new dialog widget */
  GtkWidget *pane_dialog = gtk_dialog_new_with_buttons("PANE Alert", NULL, GTK_DIALOG_MODAL, "Continue", GTK_RESPONSE_OK, "Cancel", NULL, NULL);
  gtk_widget_set_usize(pane_dialog, 400, 200);
  
  /* set up dialog content */
  GtkWidget *pane_content = gtk_dialog_get_content_area(GTK_DIALOG (pane_dialog));
  GtkWidget *pane_label = gtk_label_new(message);
  gtk_container_add(GTK_CONTAINER (pane_content), pane_label);
 
  /* throw up the dialog */
  gtk_widget_show(pane_label);
  gtk_widget_show(pane_content);
  gint response = gtk_dialog_run(GTK_DIALOG (pane_dialog));

  gtk_widget_destroy(pane_dialog);

  return response;
}


/* Returns 1 on success, 0 on failure.
   If 1 is returned, then the beginning of the reservation to be made is returned
   through out_time.
 */
static int
calculate_open_time(char *schedbuf, int call_length, int *out_time)
{
  /* get number of entries */
  int numentries;
  sscanf(schedbuf, "%i; %s", &numentries, schedbuf);

  /* loop through looking for possible reservations */
  /* NOTE: tokens will become obselete and will not be used here */
  /* NOTE: time is measured in seconds */
  int i, time_begin, capacity, tokens;
  int time_begin_start = -1;
  int found = 0;
  int bytesread = 0;
  for(i = 0; i < numentries && found != 1 && bytesread < 256; i++)
  { 
	/* inf signifies the total bandwidth in the system, not be used for a reservation */
	if(strncmp("inf", schedbuf, 3) != 0)
	{
  		sscanf(schedbuf + bytesread, "%i,%i,%i; ", &time_begin, &capacity, &tokens);	
		
		if(time_begin_start != -1 && (time_begin - time_begin_start) >= call_length)
		{
			found = 1;
		}
		else
		{
			if(capacity >= RESERVATION)
			{
				if(time_begin_start == -1)
				{
					time_begin_start = time_begin;
					if(i + 1 == numentries)
					{
						found = 1;
					}
				}
				else
				{
					if((time_begin - time_begin_start) >= call_length)
					{
						found = 1;
					}
				}
			}
			else
			{
				time_begin_start = -1;	
			}
		}
	}

	while((*(schedbuf + bytesread)) != ' ') bytesread++;
	bytesread++;
  }

  *out_time = time_begin_start;

  return found;
}


/* gets the reservation schedule and mediates between user and server to find reservation.
   Returns GTK_RESPONSE_OK if call should be place.
   Returns NULL if call should not go through.
*/
static int
check_for_timeslot(int sockfd, int call_length)
{
  int numbytes;

  /* get reservation schedule */
  if((numbytes = send(sockfd, getsched, strlen(getsched), 0)) == -1)
  {
	  fprintf(stdout, "Could not send to PANE server.\n");
	  return NULL;
  }

  char schedbuf[SCHEDBUFSIZE];
  if((numbytes = recv(sockfd, schedbuf, SCHEDBUFSIZE - 1, 0)) == -1)
  {
	  fprintf(stdout, "Could not receive from PANE server.\n");
	  return NULL;
  }
  schedbuf[numbytes] = '\0';

  int time_begin;
  int found = calculate_open_time(schedbuf, call_length, &time_begin);

  /* no reservation avalible */
  if(found == 0)
  {
   	char makeresstring[256];
	sprintf(makeresstring, "A reservation for a %i minute call could not be made.\nWould you like to place the call anyways?", call_length/60);
	return query_user_generic2(makeresstring);
  }

  /* at this point, time_begin and capacity are set to correct values */
  
  /* check if the reservation should be made */
   char makeresstring[256];
   sprintf(makeresstring, "A reservation at %i is avalible for a %i minute call.\nWould you like to reserve this call, make the call now regardless of quality or cancel the call?", time_begin, call_length/60);

  /* create new dialog widget */
  GtkWidget *dialog = gtk_dialog_new_with_buttons("PANE Alert", NULL, GTK_DIALOG_MODAL, "Reserve", GTK_RESPONSE_YES, "Call Now", GTK_RESPONSE_OK, "Cancel", NULL, NULL);
  gtk_widget_set_usize(dialog, 700, 400);
  
  /* set up dialog content */
  GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG (dialog));
  GtkWidget *label = gtk_label_new(makeresstring);
  gtk_container_add(GTK_CONTAINER (content), label);
 
  /* throw up the dialog */
  gtk_widget_show(label);
  gtk_widget_show(content);
  gint response = gtk_dialog_run(GTK_DIALOG (dialog));

  gtk_widget_destroy(dialog);

  if(response == GTK_RESPONSE_YES)
  {
	  char reservation[256];
	  sprintf(reservation, "reserve(user=Jordan) = 1000000 on forJordan from %i to %i.", time_begin, time_begin + call_length);

	  /* make reservation */
	  if((numbytes = send(sockfd, reservation, strlen(reservation), 0)) == -1)
	  {
		  fprintf(stdout, "Could not send to PANE server.\n");
		  return NULL;
	  }

	  char buf[12]; /* this should only end up reading "True" or "False" */
	  if((numbytes = recv(sockfd, buf, 12, 0)) == -1)
	  {
		  fprintf(stdout, "Could not receive from PANE server.\n");
		  return NULL;
	  }


	  if(strncmp("True", buf, 4) != 0)
	  {
		  /* reservation could not be made */
		  return query_user_generic2("A reservation for a call of that length could not be made.\nWould you like to place the call anyways?");
		
	  }

	  return NULL;
  }
  
  return response;
}


/* the exposed function. returns GTK_RESPONSE_OK iff a reservation is successfully made or has been made for the current time. */
gint
reserve_bandwidth()
{

  /* TODO: Don't open a socket every time? */
  /* reserve bandwidth */
  int numbytes, sockfd;

  if((sockfd = connect_to_pane(SERVERHOST, SERVERPORT)) == -1)
  {
	  return NULL;
  }

  /* attempt to reserve for now */
  if((numbytes = send(sockfd, user, strlen(user), 0)) == -1)
  {
	  close(sockfd);
	  fprintf(stdout, "Could not send to PANE server.\n");
	  return NULL;
  }

  if((numbytes = send(sockfd, reserve, strlen(reserve), 0)) == -1)
  {
	  close(sockfd);
	  fprintf(stdout, "Could not send to PANE server.\n");
	  return NULL;
  }

  /* TODO: Should probably timeout? */
  char buf[12]; /* this should only end up reading "True" or "False" */
  if((numbytes = recv(sockfd, buf, 12, 0)) == -1)
  {
	  close(sockfd);
	  fprintf(stdout, "Could not receive from PANE server.\n");
	  return NULL;
  }
  
  if(strncmp("True", buf, 4) != 0)
  {
	  /* reservation could not be made */
	  int response = check_for_timeslot(sockfd, query_user_call_length() * 60);

  	  close(sockfd);
	  return response;
  }

  /* rservation successfully made */
  close(sockfd);
  return GTK_RESPONSE_OK;
}
