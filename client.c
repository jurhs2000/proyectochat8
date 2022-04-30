/**
 * @file client.c
 * @author Julio H., Oscar S.
 * @brief 
 * @version 0.1
 * 
 * Universidad del Valle de Guatemala
 * Sistemas Operativos - CC3064
 * Proyecto - Chat
 */
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <gtk/gtk.h>

GtkWidget *window;
GtkWidget *grid;

static void showUsers(void) {
    int usersSize = 10;
    char labelText[50];
    GtkWidget *label;
    for (int i=0; i < usersSize; ++i) {
        snprintf(labelText, 50, "Este es el usuario %d", i);
        label = gtk_label_new(labelText);
        gtk_grid_attach (GTK_GRID (grid), label, 0, i+1, 2, 1);
    }
}

static void cleanUsers(GtkWidget *widget, gpointer data) {
    int usersSize = 10;
    for (int i=0; i < usersSize; ++i) {
        gtk_grid_remove_row(GTK_GRID (grid), 1);
    }
}

static void show_message() {
    GtkWidget *dialog;
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL;
    char error[] = "error1";
    dialog = gtk_message_dialog_new (NULL,
                                    flags,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    "Error reading “%s”: %s",
                                    error,
                                    g_strerror (2));
    g_signal_connect (dialog, "response",
                        G_CALLBACK (gtk_window_destroy),
                        NULL);
    gtk_widget_show(dialog);
}

static void button_action(GtkWidget *widget, gpointer data)
{
    g_print("Button clicked\n");
    showUsers();
}

static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *showUsersButton, *switchStatusButton;

    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Window");
    gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

    grid = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(window), grid);

    showUsersButton = gtk_button_new_with_label ("Show Users");
    g_signal_connect (showUsersButton, "clicked", G_CALLBACK (button_action), NULL);
    gtk_grid_attach (GTK_GRID (grid), showUsersButton, 0, 0, 1, 1);

    switchStatusButton = gtk_button_new_with_label ("Switch Status");
    g_signal_connect (switchStatusButton, "clicked", G_CALLBACK (cleanUsers), NULL);
    gtk_grid_attach (GTK_GRID (grid), switchStatusButton, 1, 0, 1, 1);

    gtk_window_present (GTK_WINDOW (window));
}

int main(int argc, char *argv[])
{
    int socket_desc;
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    char *message , server_reply[2000];
    if (socket_desc < 0)
    {
        perror("ERROR opening socket");
        return -1;
    }
    // Creating server
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8888);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    // Connecting to server
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("\n Error! \n");
        return 1;
    }

    puts("Connected\n");
	
	//Send some data
	message = "adfsgdsgdfgdgfdgd dfg dfg d gfd gf dg df gd fg dg d fgd gf d gdfgdgg";
	if( send(socket_desc , message , strlen(message) , 0) < 0)
	{
		puts("Send failed");
		return 1;
	}
	puts("Data Send\n");

    //Receive a reply from the server
	if( recv(socket_desc, server_reply , 2000 , 0) < 0)
	{
		puts("recv failed");
	}
	puts("Reply received\n");
	puts(server_reply);

    // Displaying window with GTK
    GtkApplication *app;
    int status;

    app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

    return status;
}
