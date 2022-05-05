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
#include <pthread.h>

int socket_desc, status = 0;
char *target = "all";
char *user;
int pipefd[2];
GtkWidget *window;
GtkWidget *grid;

static void show_message(char message[]) {
    GtkWidget *dialog;
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL;
    dialog = gtk_message_dialog_new (NULL, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                    "%s",
                                    message);
    g_signal_connect (dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
    gtk_widget_show(dialog);
}

static void show_error(char error[]) {
    GtkWidget *dialog;
    GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL;
    dialog = gtk_message_dialog_new (NULL, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                                    "Error: %s",
                                    error);
    g_signal_connect (dialog, "response", G_CALLBACK (gtk_window_destroy), NULL);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
    gtk_widget_show(dialog);
}

static void csr(char *response) {
    memmove(response, response+1, strlen(response)-2);
    response[strlen(response)-1] = '\0';
    response[strlen(response)-1] = '\0';
}

static int send_message(GtkWidget *widget, GtkEntryBuffer *buffer)
{
    const gchar *entry_text;
    entry_text = gtk_entry_buffer_get_text(buffer); //Send init connection data
    // Creating JSON object
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
    char *timestamp;
    time(&seconds);
    timeinfo = localtime(&seconds);
    timestamp = asctime(timeinfo);
    timestamp[strlen(timestamp)-1] = '\0';
    delivered_at = cJSON_CreateString(timestamp);
    from = cJSON_CreateString(user);
    message = cJSON_CreateString(entry_text);
    to = cJSON_CreateString(target);
    cJSON_AddItemToArray(body, message);
    cJSON_AddItemToArray(body, from);
    cJSON_AddItemToArray(body, delivered_at);
    cJSON_AddItemToArray(body, to);
    cJSON_AddItemToObject(post_chat, "body", body);
    // Parsing JSON to string
    char *post_chat_str;
    post_chat_str = cJSON_Print(post_chat);
    puts(post_chat_str);
    // Sending connection request
	if( send(socket_desc , post_chat_str , strlen(post_chat_str) , 0) < 0)
	{
		puts("Message can not be sended");
		return 1;
	}
	puts("Message sended successfully\n");
    // Clean buffer
    gtk_entry_buffer_set_text(buffer, "", 0);
}

static void get_chat(char *chat_str) {
    GtkWidget *scrolled, *view;
    scrolled = gtk_scrolled_window_new();
    view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view), 10);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(view), 10);
    GtkTextBuffer *text_buffer = gtk_text_buffer_new(NULL);
    GtkTextIter iter;
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(view), text_buffer);
    gtk_text_buffer_get_iter_at_offset(text_buffer, &iter, 0);
    puts(chat_str);
    const cJSON *messages = NULL;
    const cJSON *message = NULL;
    messages = cJSON_Parse(chat_str);
    char *property_string = NULL;
    char *message_formatted = (char *) malloc(8);
    strcpy(message_formatted, "");
    cJSON_ArrayForEach(message, messages) {
        message_formatted = (char *) realloc(message_formatted, strlen(message_formatted) + 512);
        cJSON *user = cJSON_GetArrayItem(message, 1);
        property_string = cJSON_Print(user);
        csr(property_string);
        strcat(message_formatted, property_string);
        cJSON *delivTime = cJSON_GetArrayItem(message, 2);
        property_string = cJSON_Print(delivTime);
        csr(property_string);
        strcat(message_formatted, " sended at ");
        strcat(message_formatted, property_string);
        cJSON *text = cJSON_GetArrayItem(message, 0);
        property_string = cJSON_Print(text);
        csr(property_string);
        strcat(message_formatted, ": ");
        strcat(message_formatted, property_string);
        strcat(message_formatted, "\n");
    }
    gtk_text_buffer_insert(text_buffer, &iter, message_formatted, -1);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), view);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled), 600);
    gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(scrolled), 800);
    gtk_grid_attach(GTK_GRID(grid), scrolled, 0,2,3,1);
}

static void show_help(GtkWidget *widget, gpointer data) {
    show_message("Authors: Julio H. & Oscar S.\n\nCHATROOM\n\n - Funciona en conjunto al servidor\n - Ingresa tu nombre de usuario ip del servidor y puerto a conectarse como parametros de ejecucion del programa\n - Presiona los botones de acciones para realizar una peticion al servidor y espera la respuesta\n - Para cambiar tu estado a 'inactivo' presiona 'Exit'");
}

static void show_user_info(char *user_ip, char *user_status)
{
    csr(user_ip);
    csr(user_status);
    printf("\nip del usuario: %s", user_ip);
    printf("\nstatus del usuario: %s\n", user_status);
    //show_message(user_ip);
}

static int req_user_info(GtkWidget *widget, char *username) {
    // Creating JSON object
    cJSON *get_user = cJSON_CreateObject();
    cJSON *request = NULL;
    cJSON *body = NULL;
    request = cJSON_CreateString("GET_USER");
    cJSON_AddItemToObject(get_user, "request", request);
    body = cJSON_CreateString(username);
    cJSON_AddItemToObject(get_user, "body", body);
    // Parsing JSON to string
    char *get_user_str;
    get_user_str = cJSON_Print(get_user);
    // Sending connection request
	if( send(socket_desc , get_user_str , strlen(get_user_str) , 0) < 0)
	{
		puts("User Info can not be requested");
		return 1;
	}
	puts("User Info requested successfully\n");
}

static int req_chat_user(GtkWidget *widget,  char *username) {
    // Creating JSON object
    cJSON *get_chat = cJSON_CreateObject();
    cJSON *request = NULL;
    cJSON *body = NULL;
    request = cJSON_CreateString("GET_CHAT");
    cJSON_AddItemToObject(get_chat, "request", request);
    body = cJSON_CreateString(username);
    cJSON_AddItemToObject(get_chat, "body", body);
    // Parsing JSON to string
    char *get_chat_str;
    get_chat_str = cJSON_Print(get_chat);
    // Sending connection request
	if( send(socket_desc , get_chat_str , strlen(get_chat_str) , 0) < 0)
	{
		puts("Direct Messages can not be requested");
		return 1;
	}
	puts("Direct Messages requested successfully\n");
    target = username;
}

static void show_users(char *users_list) {
    gtk_grid_remove_row(GTK_GRID (grid), 2);
    gtk_grid_remove_row(GTK_GRID (grid), 1);
    GtkWidget *scrolled, *users_grid, *label, *info_button, *chat_button;
    scrolled = gtk_scrolled_window_new();
    users_grid = gtk_grid_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), users_grid);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(scrolled), 600);
    gtk_scrolled_window_set_min_content_width(GTK_SCROLLED_WINDOW(scrolled), 800);
    gtk_grid_attach(GTK_GRID(grid), scrolled, 0,2,3,1);
    cJSON *users = cJSON_Parse(users_list);
    const cJSON *user = NULL;
    int i = 0;
    cJSON_ArrayForEach(user, users) {
        char *user_text = (char *)malloc(248), *user_text_f = (char *)malloc(248), *property_string = NULL;
        strcpy(user_text, "");
        strcpy(user_text_f, "");
        cJSON *value = cJSON_GetArrayItem(user, 0);
        property_string = cJSON_Print(value);
        csr(property_string);
        strcat(user_text, property_string);
        strcat(user_text_f, property_string);
        value = cJSON_GetArrayItem(user, 1);
        property_string = cJSON_Print(value);
        csr(property_string);
        strcat(user_text_f, " is ");
        if (strcmp(property_string, "0") == 0) {
            strcat(user_text_f, "active");
        } else {
            strcat(user_text_f, "busy");
        }

        label = gtk_label_new(user_text_f);
        gtk_widget_set_margin_start(label, 10);
        gtk_widget_set_margin_top(label, 10);
        gtk_grid_attach (GTK_GRID (users_grid), label, 0, i, 2, 1);
        info_button = gtk_button_new_with_label ("User Info");
        gtk_widget_set_margin_start(info_button, 10);
        gtk_widget_set_margin_top(info_button, 10);
        g_signal_connect (info_button, "clicked", G_CALLBACK (req_user_info), user_text);
        gtk_grid_attach (GTK_GRID (users_grid), info_button, 3, i, 1, 1);
        chat_button = gtk_button_new_with_label ("Chat User");
        gtk_widget_set_margin_start(chat_button, 10);
        gtk_widget_set_margin_top(chat_button, 10);
        g_signal_connect (chat_button, "clicked", G_CALLBACK (req_chat_user), user_text);
        gtk_grid_attach (GTK_GRID (users_grid), chat_button, 4, i, 1, 1);
        i++;
    }
}

static void manage_user_res(GtkWidget *widget, gpointer data) {
    char server_reply[2048];
    int read_res = read(pipefd[0], &server_reply, 2048);
    if (read_res < 0) {
        perror("read");
    } else {
        puts("readed from pipe");
        printf("%s\n", server_reply);
        const cJSON *body = NULL;
        cJSON *response = cJSON_Parse(server_reply);
        body = cJSON_GetObjectItemCaseSensitive(response, "body");
        cJSON *body_first = cJSON_GetArrayItem(body, 0);
        int is_arr = cJSON_IsArray(body_first);
        if (is_arr == 1) { // todos los usuarios
            char *users_list;
            users_list = cJSON_Print(body);
            show_users(users_list);
        } else { // info de usuario especifico
            cJSON *user_status = cJSON_GetArrayItem(body, 1);
            char *user_ip_str = NULL, *user_status_str = NULL;
            user_ip_str = cJSON_Print(body_first);
            user_status_str = cJSON_Print(user_status);
            show_user_info(user_ip_str, user_status_str);
        }
    }
}

static void render_general_chat(GtkWidget *widget, gpointer data) {
    gtk_grid_remove_row(GTK_GRID (grid), 2);
    gtk_grid_remove_row(GTK_GRID (grid), 1);
    GtkWidget *message_entry, *send_button;
    GtkEntryBuffer *buffer = gtk_entry_buffer_new("", 0);
    message_entry = gtk_entry_new_with_buffer (buffer);
    gtk_widget_set_margin_start(message_entry, 10);
    gtk_widget_set_margin_top(message_entry, 10);
    gtk_widget_set_margin_bottom(message_entry, 10);
    g_signal_connect(message_entry, "activate", G_CALLBACK(send_message), buffer);
    gtk_entry_set_placeholder_text(GTK_ENTRY(message_entry), "Ingresa un mensaje:");
    gtk_grid_attach (GTK_GRID (grid), message_entry, 0, 1, 2, 1);
    send_button = gtk_button_new_with_label ("Send Message");
    gtk_widget_set_margin_start(send_button, 10);
    gtk_widget_set_margin_end(send_button, 10);
    gtk_widget_set_margin_top(send_button, 10);
    gtk_widget_set_margin_bottom(send_button, 10);
    g_signal_connect (send_button, "clicked", G_CALLBACK (send_message), buffer);
    gtk_grid_attach (GTK_GRID (grid), send_button, 2, 1, 1, 1);
    char server_reply[2048];
    int read_res = read(pipefd[0], &server_reply, 2048);
    if (read_res < 0) {
        perror("read");
    } else {
        puts("readed from pipe");
        printf("%s\n", server_reply);
        const cJSON *body = NULL;
        cJSON *response = cJSON_Parse(server_reply);
        body = cJSON_GetObjectItemCaseSensitive(response, "body");
        char *chat_list;
        chat_list = cJSON_Print(body);
        get_chat(chat_list);
    }
}

static int req_chat(GtkWidget *widget, gpointer data) {
    // Creating JSON object
    cJSON *get_chat = cJSON_CreateObject();
    cJSON *request = NULL;
    cJSON *body = NULL;
    request = cJSON_CreateString("GET_CHAT");
    cJSON_AddItemToObject(get_chat, "request", request);
    body = cJSON_CreateString(target);
    cJSON_AddItemToObject(get_chat, "body", body);
    // Parsing JSON to string
    char *get_chat_str;
    get_chat_str = cJSON_Print(get_chat);
    // Sending connection request
	if( send(socket_desc , get_chat_str , strlen(get_chat_str) , 0) < 0)
	{
		puts("Messages can not be requested");
		return 1;
	}
	puts("Messages requested successfully\n");
    target = "all";
}

static int req_all_chat(GtkWidget *widget, gpointer data) {
    target = "all";
    req_chat(widget, data);
}

static int req_users(GtkWidget *widget, gpointer data) {
    // Creating JSON object
    cJSON *get_user = cJSON_CreateObject();
    cJSON *request = NULL;
    cJSON *body = NULL;
    request = cJSON_CreateString("GET_USER");
    cJSON_AddItemToObject(get_user, "request", request);
    body = cJSON_CreateString("all");
    cJSON_AddItemToObject(get_user, "body", body);
    // Parsing JSON to string
    char *get_user_str;
    get_user_str = cJSON_Print(get_user);
    // Sending connection request
	if( send(socket_desc , get_user_str , strlen(get_user_str) , 0) < 0)
	{
		puts("Users can not be requested");
		return 1;
	}
	puts("Users requested successfully\n");
}

static int req_switch_status(GtkWidget *widget, gpointer data) {
    // Creating JSON object
    cJSON *put_status = cJSON_CreateObject();
    cJSON *request = NULL;
    cJSON *body = NULL;
    request = cJSON_CreateString("PUT_STATUS");
    cJSON_AddItemToObject(put_status, "request", request);
    if (status == 0) {
        status = 2;
    } else {
        status = 0;
    }
    if (status == 0) {
        body = cJSON_CreateString("0");
    } else {
        body = cJSON_CreateString("2");
    }
    cJSON_AddItemToObject(put_status, "body", body);
    // Parsing JSON to string
    char *put_status_str;
    put_status_str = cJSON_Print(put_status);
    // Sending connection request
	if( send(socket_desc , put_status_str , strlen(put_status_str) , 0) < 0)
	{
		puts("Status can not be changed");
		return 1;
	}
	puts("Status changed successfully\n");
}

static int req_end_conex(GtkWidget *widget, gpointer data) {
    // Creating JSON object
    cJSON *end_conex = cJSON_CreateObject();
    cJSON *request = NULL;
    request = cJSON_CreateString("END_CONEX");
    // Parsing JSON to string
    char *end_conex_str;
    end_conex_str = cJSON_Print(end_conex);
    // Sending connection request
	if( send(socket_desc , end_conex_str , strlen(end_conex_str) , 0) < 0)
	{
		puts("Can not request end connection");
		return 1;
	}
	puts("End connection requested successfully\n");
    exit(0);
}

static char *switch_status_text(void) {
    char *status_text;
    if (status == 0) {
        status_text = "Switch Status to Busy";
    } else {
        status_text = "Switch Status to Active";
    }
    return status_text;
}

static void switch_status(GtkWidget *widget, gpointer data) {
    char server_reply[2048];
    int read_res = read(pipefd[0], &server_reply, 2048);
    if (read_res < 0) {
        perror("read");
    } else {
        puts("readed from pipe");
        printf("%s\n", server_reply);
        const cJSON *code = NULL;
        cJSON *response = cJSON_Parse(server_reply);
        code = cJSON_GetObjectItemCaseSensitive(response, "code");
        if (code->valueint == 200) { // TODO: no testeado
            GtkWidget *button_to_remove = gtk_grid_get_child_at(GTK_GRID(grid), 2, 0);
            gtk_grid_remove(GTK_GRID (grid), button_to_remove);
            GtkWidget *switch_status_button;
            switch_status_button = gtk_button_new_with_label (switch_status_text());
            gtk_widget_set_margin_start(switch_status_button, 10);
            gtk_widget_set_margin_end(switch_status_button, 10);
            gtk_widget_set_margin_top(switch_status_button, 10);
            gtk_widget_set_margin_bottom(switch_status_button, 10);
            g_signal_connect (switch_status_button, "clicked", G_CALLBACK (req_switch_status), NULL);
            gtk_grid_attach (GTK_GRID (grid), switch_status_button, 2, 0, 1, 1);
        }
    }
}

static void manage_post_chat(GtkWidget *widget, gpointer data) {
    char server_reply[2048];
    int read_res = read(pipefd[0], &server_reply, 2048);
    if (read_res < 0) {
        perror("read");
    } else {
        puts("readed from pipe");
        printf("%s\n", server_reply);
        const cJSON *code = NULL;
        cJSON *response = cJSON_Parse(server_reply);
        code = cJSON_GetObjectItemCaseSensitive(response, "code");
        if (code->valueint == 200) { // TODO: no testeado
            // Si el mensaje se envio bien, obtener el chat
            req_chat(NULL, NULL);
        }
    }
}

static void receive_from_server(void) {
    //Receive a reply from the server
    char *server_reply = malloc(2000);
    while (1) {
        if( recv(socket_desc, server_reply , 2000 , 0) > 0)
        {
            puts("Reply received\n");
            puts(server_reply);
            int write_res = write(pipefd[1], server_reply, strlen(server_reply));
            if (write_res < 0) {
                puts("Error writing to pipe");
            }
            const cJSON *response_type = NULL;
            cJSON *response = cJSON_Parse(server_reply);
            response_type = cJSON_GetObjectItemCaseSensitive(response, "response");
            char *response_str = NULL;
            response_str = cJSON_Print(response_type);
            csr(response_str);

            if (strcmp(response_str, "GET_CHAT") == 0) {
                // render general chat if response is of type GET_CHAT
                g_signal_emit_by_name(window, "GET_CHAT_RES");
            } else if (strcmp(response_str, "GET_USER") == 0) {
                g_signal_emit_by_name(window, "GET_USER_RES");
            } else if (strcmp(response_str, "POST_CHAT") == 0) {
                g_signal_emit_by_name(window, "POST_CHAT_RES");
            } else if (strcmp(response_str, "PUT_STATUS") == 0) {
                g_signal_emit_by_name(window, "PUT_STATUS_RES");
            } else if (strcmp(response_str, "NEW_MESSAGE") == 0) {
                g_signal_emit_by_name(window, "POST_CHAT_RES");
            } else if (strcmp(response_str, "INIT_CONEX") == 0) {
                // clean pipe
            } 
        }
    }
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
    char *connection_init;
    cJSON *init_conex = cJSON_CreateObject();
    cJSON *request = NULL;
    cJSON *body = NULL;
    cJSON *connect_time = NULL;
    cJSON *user_id = NULL;
    request = cJSON_CreateString("INIT_CONEX");
    cJSON_AddItemToObject(init_conex, "request", request);
    body = cJSON_CreateArray();
    time_t seconds;
    struct tm *timeinfo;
    time(&seconds);
    timeinfo = localtime(&seconds);
    connect_time = cJSON_CreateString(asctime(timeinfo));
    user_id = cJSON_CreateString(user);
    cJSON_AddItemToArray(body, connect_time);
    cJSON_AddItemToArray(body, user_id);
    cJSON_AddItemToObject(init_conex, "body", body);
    connection_init = cJSON_Print(init_conex);
    // Sending connection request
	if( send(socket_desc , connection_init , strlen(connection_init) , 0) < 0)
	{
		puts("Init connection failed");
		return 1;
	}
	puts("Successful init connection\n");
}

static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *show_users_button, *view_chat_button, *switch_status_button, *end_button, *help_button;

    window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (window), "Window");
    gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

    grid = gtk_grid_new();
    gtk_window_set_child(GTK_WINDOW(window), grid);

    show_users_button = gtk_button_new_with_label ("Show Users");
    gtk_widget_set_margin_start(show_users_button, 10);
    gtk_widget_set_margin_top(show_users_button, 10);
    gtk_widget_set_margin_bottom(show_users_button, 10);
    g_signal_connect (show_users_button, "clicked", G_CALLBACK (req_users), NULL);
    gtk_grid_attach (GTK_GRID (grid), show_users_button, 0, 0, 1, 1);

    view_chat_button = gtk_button_new_with_label ("View Chat");
    gtk_widget_set_margin_start(view_chat_button, 10);
    gtk_widget_set_margin_top(view_chat_button, 10);
    gtk_widget_set_margin_bottom(view_chat_button, 10);
    g_signal_connect (view_chat_button, "clicked", G_CALLBACK (req_chat), NULL);
    gtk_grid_attach (GTK_GRID (grid), view_chat_button, 1, 0, 1, 1);

    switch_status_button = gtk_button_new_with_label (switch_status_text());
    gtk_widget_set_margin_start(switch_status_button, 10);
    gtk_widget_set_margin_end(switch_status_button, 10);
    gtk_widget_set_margin_top(switch_status_button, 10);
    g_signal_connect (switch_status_button, "clicked", G_CALLBACK (req_switch_status), NULL);
    gtk_grid_attach (GTK_GRID (grid), switch_status_button, 2, 0, 1, 1);

    end_button = gtk_button_new_with_label ("Exit");
    gtk_widget_set_margin_start(end_button, 10);
    gtk_widget_set_margin_end(end_button, 10);
    gtk_widget_set_margin_top(end_button, 10);
    gtk_widget_set_margin_bottom(end_button, 10);
    g_signal_connect (end_button, "clicked", G_CALLBACK (req_end_conex), NULL);
    gtk_grid_attach (GTK_GRID (grid), end_button, 3, 0, 1, 1);

    help_button = gtk_button_new_with_label ("Help");
    gtk_widget_set_margin_start(help_button, 10);
    gtk_widget_set_margin_end(help_button, 10);
    gtk_widget_set_margin_top(help_button, 10);
    gtk_widget_set_margin_bottom(help_button, 10);
    g_signal_connect (help_button, "clicked", G_CALLBACK (show_help), NULL);
    gtk_grid_attach (GTK_GRID (grid), help_button, 4, 0, 1, 1);

    g_signal_new("GET_CHAT_RES", G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
    g_signal_connect(window, "GET_CHAT_RES", G_CALLBACK(render_general_chat), NULL);
    g_signal_new("GET_USER_RES", G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
    g_signal_connect(window, "GET_USER_RES", G_CALLBACK(manage_user_res), NULL);
    g_signal_new("POST_CHAT_RES", G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
    g_signal_connect(window, "POST_CHAT_RES", G_CALLBACK(manage_post_chat), NULL);
    g_signal_new("PUT_STATUS_RES", G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, NULL, NULL, g_cclosure_marshal_VOID__STRING, G_TYPE_NONE, 1, G_TYPE_STRING);
    g_signal_connect(window, "PUT_STATUS_RES", G_CALLBACK(manage_user_res), NULL);

    gtk_window_present (GTK_WINDOW (window));
    // define gl_context
    GdkGLContext *gl_context = gdk_gl_context_get_current();
    gdk_gl_context_make_current(gl_context);

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    pthread_t thread;
    pthread_create(&thread, NULL, (void *)&receive_from_server, NULL);
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
