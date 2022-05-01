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
#include <time.h>
#include <cjson/cJSON.h>

int socket_desc;
char *user;
GtkWidget *window;
GtkWidget *grid;

static void show_message(char error[]) {
    GtkWidget *dialog;
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL;
    dialog = gtk_message_dialog_new (NULL,
                                    flags,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    "Error: %s",
                                    error);
    g_signal_connect (dialog, "response",
                        G_CALLBACK (gtk_window_destroy),
                        NULL);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
    gtk_widget_show(dialog);
}

static void receive_from_server() {
    //Receive a reply from the server
    char server_reply[2000];
	if( recv(socket_desc, server_reply , 2000 , 0) < 0)
	{
		puts("recv failed");
	}
	puts("Reply received\n");
    show_message(server_reply);
}

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

static void button_action(GtkWidget *widget, gpointer data)
{
    g_print("Button clicked\n");
    showUsers();
    show_message("hoaaa");
}

static int send_message(GtkWidget *widget, GtkEntryBuffer *buffer)
{
    const gchar *entry_text;
    entry_text = gtk_entry_buffer_get_text(buffer);//Send init connection data
    char *post_chat_str;
    cJSON *post_chat = cJSON_CreateObject();
    cJSON *request = NULL;
    cJSON *body = NULL;
    cJSON *message = NULL;
    cJSON *from = NULL;
    cJSON *delivered_at = NULL;
    cJSON *to = NULL;
    request = cJSON_CreateString("POST_CHAT");
    cJSON_AddItemToObject(post_chat, "request", request);
    body = cJSON_CreateArray();
    time_t seconds;
    struct tm *timeinfo;
    time(&seconds);
    timeinfo = localtime(&seconds);
    delivered_at = cJSON_CreateString(asctime(timeinfo));
    from = cJSON_CreateString(user);
    message = cJSON_CreateString(entry_text);
    to = cJSON_CreateString("all");
    cJSON_AddItemToArray(body, delivered_at);
    cJSON_AddItemToArray(body, from);
    cJSON_AddItemToArray(body, message);
    cJSON_AddItemToArray(body, to);
    cJSON_AddItemToObject(post_chat, "body", body);
    post_chat_str = cJSON_Print(post_chat);
    // Sending connection request
	if( send(socket_desc , post_chat_str , strlen(post_chat_str) , 0) < 0)
	{
		puts("Message can not be sended");
		return 1;
	}
	puts("Message sended successfully\n");
    gtk_entry_buffer_set_text(buffer, "", 0);
}

static int create_connection(char *argv[]) {
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc < 0)
    {
        printf("\n Error: Can not create the socket \n");
        return -1;
    }
    // Creating server
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[3]));
    server.sin_addr.s_addr = inet_addr(argv[2]);
    // Connecting to server
    if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("\n Error: Can not connect to server \n");
        return 1;
    }
    puts("Connected\n");
	//Send init connection data
    char *connectionInit;
    cJSON *initConex = cJSON_CreateObject();
    cJSON *request = NULL;
    cJSON *body = NULL;
    cJSON *connect_time = NULL;
    cJSON *user_id = NULL;
    request = cJSON_CreateString("INIT_CONEX");
    cJSON_AddItemToObject(initConex, "request", request);
    body = cJSON_CreateArray();
    time_t seconds;
    struct tm *timeinfo;
    time(&seconds);
    timeinfo = localtime(&seconds);
    connect_time = cJSON_CreateString(asctime(timeinfo));
    user_id = cJSON_CreateString(user);
    cJSON_AddItemToArray(body, connect_time);
    cJSON_AddItemToArray(body, user_id);
    cJSON_AddItemToObject(initConex, "body", body);
    connectionInit = cJSON_Print(initConex);
    // Sending connection request
	if( send(socket_desc , connectionInit , strlen(connectionInit) , 0) < 0)
	{
		puts("Init connection failed");
		return 1;
	}
	puts("Successful init connection\n");
}

static void render_general_chat(void) {
    GtkWidget *message_entry, *send_button;
    GtkEntryBuffer *buffer = gtk_entry_buffer_new("", 0);
    message_entry = gtk_entry_new_with_buffer (buffer);
    g_signal_connect(message_entry, "activate", G_CALLBACK(send_message), buffer);
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Ingresa un mensaje:");
    gtk_grid_attach (GTK_GRID (grid), message_entry, 0, 1, 2, 1);
    //send_button = gtk_button_new_with_label ("Send Message");
    //g_signal_connect (send_button, "clicked", G_CALLBACK (send_message), buffer);
    //gtk_grid_attach (GTK_GRID (grid), send_button, 3, 1, 1, 1);
}

static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *showUsersButton, *switchStatusButton, *scrolled, *view;

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

    render_general_chat();

    scrolled = gtk_scrolled_window_new();
    view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), view);
    gtk_grid_attach(GTK_GRID(grid), scrolled, 0,3,1,1);

    gtk_window_present (GTK_WINDOW (window));

    puts("Listening to server\n");
    receive_from_server();
}

int main(int argc, char *argv[])
{
    user = argv[1];
    create_connection(argv);
    // Displaying window with GTK
    GtkApplication *app;
    int status;
    app = gtk_application_new ("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
    status = g_application_run (G_APPLICATION (app), 0, NULL);
    return status;
}
