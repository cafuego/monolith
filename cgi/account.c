/* $Id$
 *
 * Account is the script that allows users to create a new acount
 * via the web interface.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <strings.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "monolith.h"
#include "libmono.h"
#include "cgi.h"
#include "registration.h"

/* declarations */
typedef struct {
    char *name;
    char *val;
} entry;

/* prototypes */

void parse_cgi_input(void);
void url_name(char *string);
void parse_input(void);
void print_results(user_t *);
int check_content(const char *, const char *);
int check_validity(void);
int error(int);
void error_2(user_t * user);
int send_www_key(const user_t *);
user_t *newuser_makesupp(void);
void fill_out_fields(user_t * user);
void format_fields(user_t * user);
void generate_new_key(user_t * user);
int main(int, char **);
int send_it(const user_t *user, char *command);

/* global vars */
int number_of_entries = 0;
char username[L_USERNAME + 1];
char password[14];
char verpassword[14];
char name[RGnameLEN + 1];
char url[RGurlLEN + 1];
char email[RGemailLEN + 1];
char street[RGaddrLEN + 1];
char zipcode[RGzipLEN + 1];
char city[RGcityLEN + 1];
char state[RGstateLEN +1];
char country[RGstateLEN + 1];
char phone[RGphoneLEN + 1];
entry entries[100];
char *referer;
int hiddeninfo = 0, showemail = 0, showurl = 0;

/* defines */
#define VALIDATION_EMAIL	BBSDIR "/share/messages/validation.txt"
#define USERNAME	1
#define EXISTS		10
#define PASSWORD	42
#define NAME		2
#define EMAIL		3
#define URL		4
#define STREET		5
#define ZIPCODE		6
#define CITY		7
#define COUNTRY		8
#define PHONE		9
#define STATE		10

void
parse_cgi_input()
{
    int m, x;
    int cl;

    referer = getenv("HTTP_REFERER");

    if (strcmp(getenv("REQUEST_METHOD"), "POST") == 0) {
	cl = atoi(getenv("CONTENT_LENGTH"));

	if (cl == 0) {
	    number_of_entries = 0;
	    return;
	}
	for (x = 0; cl && (!feof(stdin)); x++) {
	    m = x;
	    entries[x].val = fmakeword(stdin, '&', &cl);
	    plustospace(entries[x].val);
	    unescape_url(entries[x].val);
	    entries[x].name = makeword(entries[x].val, '=');
	    number_of_entries = x + 1;
	}
    } else {
	number_of_entries = 0;
	return;
    }
    return;
}

/* this function, parses the entries[] array, that is now filled
 * with information to some useful data */
void
parse_input()
{
    unsigned int i;

    /* default values, for security */
    strcpy(username, "");
    strcpy(password, "");
    strcpy(verpassword, "");
    strcpy(name, "");
    strcpy(url, "");
    strcpy(email, "");
    strcpy(street, "");
    strcpy(zipcode, "");
    strcpy(city, "");
    strcpy(country, "");
    strcpy(state, "");
    strcpy(phone, "");

    /* we have put everything in the entries now, now start parsing 
     * * those */
    for (i = 0; i < number_of_entries; i++) {
	if (EQ(entries[i].name, "username")) {
	    strncpy(username, entries[i].val, L_USERNAME);
	} else if (EQ(entries[i].name, "password")) {
	    strncpy(password, entries[i].val, 13);
	} else if (EQ(entries[i].name, "verpassword")) {
	    strncpy(verpassword, entries[i].val, 13);
	} else if (EQ(entries[i].name, "name")) {
	    strncpy(name, entries[i].val, RGnameLEN);
	    name[RGnameLEN] = '\0';
	} else if (EQ(entries[i].name, "url")) {
	    strncpy(url, entries[i].val, RGurlLEN);
	    url[RGurlLEN] = '\0';
	} else if (EQ(entries[i].name, "email")) {
	    strncpy(email, entries[i].val, RGemailLEN);
	    email[RGemailLEN] = '\0';
	} else if (EQ(entries[i].name, "street")) {
	    strncpy(street, entries[i].val, RGaddrLEN);
	    street[RGaddrLEN] = '\0';
	} else if (EQ(entries[i].name, "zipcode")) {
	    strncpy(zipcode, entries[i].val, RGzipLEN);
	    zipcode[RGzipLEN] = '\0';
	} else if (EQ(entries[i].name, "city")) {
	    strncpy(city, entries[i].val, RGcityLEN);
	    city[RGcityLEN] = '\0';
	} else if (EQ(entries[i].name, "state")) {
	    strncpy(state, entries[i].val, RGstateLEN);
	    country[RGstateLEN] = '\0';
	} else if (EQ(entries[i].name, "country")) {
	    strncpy(country, entries[i].val, RGstateLEN);
	    country[RGstateLEN] = '\0';
	} else if (EQ(entries[i].name, "phone")) {
	    strncpy(phone, entries[i].val, RGphoneLEN);
	    phone[RGphoneLEN] = '\0';
	} else if (EQ(entries[i].name, "hiddeninfo")) {
	    if (EQ(entries[i].val, "on"))
		hiddeninfo = 1;
	} else if (EQ(entries[i].name, "showemail")) {
	    if (EQ(entries[i].val, "on"))
		showemail = 1;
	} else if (EQ(entries[i].name, "showurl")) {
	    if (EQ(entries[i].val, "on"))
		showurl = 1;
	}
    }
    return;
}

void
print_results(user_t * user)
{

    printf("<html>\n<head>\n");
    printf("<title>Welcome New User</title>\n");
    printf("<link rel=\"StyleSheet\" href=\"/include/style.css\" type=\"text/css\">\n");
    printf("</head>\n");
    printf("<body bgcolor=\"#FFFFFF\" text=\"#000000\" link=\"#008740\" alink=\"#00FFFF\" vlink=\"#008F93\">\n");
    printf("<center>\n<table border=\"0\" width=\"400\">\n");
    printf("<tr><td align=\"left\" valign=\"top\">\n");
    printf("<h3 align=\"center\">Welcome to Monolith, %s!</h3>\n", user->username);
    printf("<p align=\"left\">\n");
    printf("You account has been created and a registration key has been sent to the following address: <b>%s &lt;%s&gt;</b>.\n", user->RGname, user->RGemail);
    printf("<p align=\"left\">\n");
    printf("Do read this email, as it contains the instructions on how to verify your account.<br>If you do not receive this key within the next 24 hours, please contact <a href=\"webmaster@cal044202.student.utwente.nl\">webmaster@cal044202.student.utwente.nl</a>. Include your username and account details in your email, as we cannot help you otherwise.\n");
    printf("</td></tr>\n");
    printf("</table>\n");
    printf("</center>\n");
    printf("<p>\n<hr size=\"1\">\n");
    printf("<p align=\"center\">\n");
    printf("<font size=\"-1\">&copy; <a href=\"/\">The Monolith Community</a> 1997 -- For questions/remarks email: <a href=\"mailto:webmaster@cal044202.student.utwente.nl\">webmaster@cal044202.student.utwente.nl</a><br>\n");
    printf("</font>\n</body>\n</html>\n");

    return;
}

int
main(int argc, char **argv)
{

    user_t *user = NULL;

    printf("Content-type: text/html%c%c", 10, 10);

    parse_cgi_input();
    parse_input();
    if (check_validity() == -1)
	return 1;
    user = newuser_makesupp();
    fill_out_fields(user);
    format_fields(user);
    if (check_user(user->username) == TRUE) {
	error_2(user);
	return 1;
    }
    generate_new_key(user);
    if (send_www_key(user) != 0)
	return 1;
    print_results(user);
    writeuser(user, 0);
    return 0;
}

int
check_content(const char *string, const char *validchars)
{

    size_t accepted;

    accepted = strspn(string, validchars);

    if (accepted < strlen(string)) {
	return -1;
    } else {
	return 0;
    }
}

int
check_validity()
{

    if ((check_content(username, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ") == -1)
	|| (username[1] == ' '))
	return error(USERNAME);
    if ((strcmp(password, verpassword) != 0)
	|| strlen(password) < 6)
	return error(PASSWORD);
    if ((check_content(name, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLNMOPQRSTUVWXYZ -'.,") == -1)
	|| strlen(name) < 6)
	return error(NAME);
    if ((check_content(street, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLNMOPQRSTUVWXYZ -'.,/#1234567890") == -1)
	|| strlen(street) < 6)
	return error(STREET);
    if ((check_content(zipcode, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890") == -1)
	|| strlen(zipcode) < 4)
	return error(ZIPCODE);
    if ((check_content(city, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ ,.-'") == -1)
	|| strlen(city) < 6)
	return error(CITY);
    if ((check_content(state, "abcdefghijlkmnopqrstuvwxyzABDCEFGHIJKLMNOPQRSTUVWXYZ .-,") == -1)
	|| strlen(state) < 2)
	return error(STATE);
    if ((check_content(country, "abcdefghijlkmnopqrstuvwxyzABDCEFGHIJKLMNOPQRSTUVWXYZ .-,") == -1)
	|| strlen(country) < 4)
	return error(COUNTRY);
    if ((check_content(phone, "1234567890- +()noneaNONEA/.") == -1)
	|| strlen(phone) < 3)
	return error(PHONE);
    if ((check_content(email, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@1234567890_%.-") == -1)
	|| strlen(email) < 8)
	return error(EMAIL);
    if (check_content(url, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890:/?%&=_-~@.") == -1)
	return error(URL);
    return 0;
}

int
error(int i)
{


    printf("<html>\n<head>\n");
    printf("<title>Error during account creation</title>\n");
    printf("<link rel=\"StyleSheet\" href=\"/include/style.css\" type=\"text/css\">\n");
    printf("</head>\n");
    printf("<body bgcolor=\"#FFFFFF\" text=\"#000000\" link=\"#008740\" alink=\"#00FFFF\" vlink=\"#008F93\">\n");

    switch (i) {

	case USERNAME:
	    printf("<h3 align=\"center\">Invalid username</h3>\n");
	    printf("<p align=\"left\">\n");
	    printf("The username <b>%s</b> that you entered contains invalid characters. Please make sure that the name does not start with a space or contains any special characters.\n", username);
	    printf("<p align=\"left\">\n");
	    printf("Go <a href=\"%s\">back</a> to the registration form.\n", referer);
	    break;

	case PASSWORD:
	    printf("<h3 align=\"center\">Invalid password</h3>\n");
	    printf("<p align=\"left\">\n");
	    printf("You entered an invalid password or the passwords you typed didn't match. Please make sure it is at least 6 characters long.\n");
	    printf("<p align=\"left\">\n");
	    printf("Go <a href=\"%s\">back</a> to the registration form.\n", referer);
	    break;

	case NAME:
	    printf("<h3 align=\"center\">Invalid name</h3>\n");
	    printf("<p align=\"left\">\n");
	    printf("The name <b>%s</b> you entered contains invalid characters.\n", name);
	    printf("<p align=\"left\">\n");
	    printf("Go <a href=\"%s\">back</a> to the registration form.\n", referer);
	    break;

	case STREET:
	    printf("<h3 align=\"center\">Invalid street</h3>\n");
	    printf("<p align=\"left\">\n");
	    printf("The street address <b>%s</b> you entered contains invalid characters.\n", street);
	    printf("<p align=\"left\">\n");
	    printf("Go <a href=\"%s\">back</a> to the registration form.\n", referer);
	    break;

	case ZIPCODE:
	    printf("<h3 align=\"center\">Invalid zipcode</h3>\n");
	    printf("<p align=\"left\">\n");
	    printf("The zipcode <b>%s</b> you entered contains invalid characters\n", zipcode);
	    printf("<p align=\"left\">\n");
	    printf("Go <a href=\"%s\">back</a> to the registration form.\n", referer);
	    break;

	case CITY:
	    printf("<h3 align=\"center\">Invalid city</h3>\n");
	    printf("<p align=\"left\">\n");
	    printf("The city <b>%s</b> you entered contains invalid characters.\n", city);
	    printf("<p align=\"left\">\n");
	    printf("Go <a href=\"%s\">back</a> to the registration form.\n", referer);
	    break;

	case STATE:
	    printf("<h3 align=\"center\">Invalid state/province</h3>\n");
	    printf("<p align=\"left\">\n");
	    printf("The state or province <b>%s</b> you entered contains invalid characters.\n", state);
	    printf("<p align=\"left\">\n");
	    printf("Go <a href=\"%s\">back</a> to the registration form.\n", referer);
	    break;

	case COUNTRY:
	    printf("<h3 align=\"center\">Invalid country</h3>\n");
	    printf("<p align=\"left\">\n");
	    printf("The country <b>%s</b> you entered contains invalid characters.\n", country);
	    printf("<p align=\"left\">\n");
	    printf("Go <a href=\"%s\">back</a> to the registration form.\n", referer);
	    break;

	case PHONE:
	    printf("<h3 align=\"center\">Invalid phone number</h3>\n");
	    printf("<p align=\"left\">\n");
	    printf("The phone number <b>%s</b> you entered contains invalid characters.\n", phone);
	    printf("<p align=\"left\">\n");
	    printf("Go <a href=\"%s\">back</a> to the registration form.\n", referer);
	    break;

	case EMAIL:
	    printf("<h3 align=\"center\">Invalid email address</h3>\n");
	    printf("<p align=\"left\">\n");
	    printf("The email address <b>%s</b> you entered contains invalid characters. Please make sure you enter it correctly, as your account can not be verified without it.\n", email);
	    printf("<p align=\"left\">\n");
	    printf("Go <a href=\"%s\">back</a> to the registration form.\n", referer);
	    break;

	case URL:
	    printf("<h3 align=\"center\">Invalid url</h3>\n");
	    printf("<p align=\"left\">\n");
	    printf("The url <b>%s</b> you entered contains invalid characters.\n", url);
	    printf("<p align=\"left\">\n");
	    printf("Go <a href=\"%s\">back</a> to the registration form.\n", referer);
	    break;

	default:
	    printf("<h3 align=\"center\">Unknown error</h3>\n");
	    printf("<p align=\"left\">\n");
	    break;

    }

    printf("<p align=\"left\">\n");
    printf("If you encounter any strange errors, please <a href=\"mailto:webmaster@monolith.student.utwente.nl\">let us know</a>.\n");
    printf("<p>\n<hr size=\"1\">\n");
    printf("<p align=\"center\">\n");
    printf("<font size=\"-1\">&copy; <a href=\"/\">The Monolith Community</a> 1997 -- For questions/remarks email: <a href=\"mailto:webmaster@cal044202.student.utwente.nl\">webmaster@cal044202.student.utwente.nl</a><br>\n");
    printf("</font>\n</body>\n</html>\n");
    return -1;
}
int
send_www_key(const user_t * user)
{

    char cmd[200];
    char work[80];
    FILE *fp;
    char string[L_USERNAME];

    umask(022);
    sprintf(work, BBSDIR "tmp/webemail.%d", getpid());

    sprintf(cmd, "/bin/mail -s \"Monolith Registration for %s\" \'%s\' < %s"
	    ,user->username, user->RGemail, work);

    log_it("web/DEBUG_LOG", "Command was: %s", cmd);

    fp = xfopen(work, "w", FALSE);

    if (fp == NULL) {
	log_it("web/email", "Error trying to send validation key to %s at %s.\nReason: Couldn't open tempfile: %s.", user->username, user->RGemail, work);
	unlink(work);
	return 2;
    }
    strcpy(string, user->username);

    url_name(string);

    fprintf(fp, "\nHi %s!\n\nThank you for creating an account on Monolith BBS.\n\n", user->username);
    fprintf(fp, "To complete the account creation process, you can register\n");
    fprintf(fp, "by simply pointing your web browser to:\n\n");
    fprintf(fp, "http://monolith.student.utwente.nl/cgi-bin/register?name=%s&key=%ld\n\n", string, user->validation_key);
    fprintf(fp, "Visiting this page will automatically register your account.\n\n");
    fprintf(fp, "After this, you can log on to the BBS as %s using your password.\n\n", user->username);
    fprintf(fp, "We are looking forward to seeing you on Monolith BBS!\n\n");
    fprintf(fp, "- The Admin of Monolith BBS.\n");

    fclose(fp);
    
    (void)send_it(user, cmd);
    unlink(work);
    return 0;
}

int
send_it (const user_t *user, char *command)
{
    int pid = 0;
    char *argv[4];

    if (command == 0) {
        log_it("web/email", "No command");
        return 1;
    }
    pid = fork();
    printf("<!-- DEBUG: fork() = %d -->\n", pid);
    if (pid == -1) {
	log_it("web/email", "Error trying to send validation key to %s.\nReason: Unable to fork()", user->username);
        return -1;
    }
    if (pid == 0) {
        argv[0] = "sh";
        argv[1] = "-c";
        argv[2] = command;
        argv[3] = 0;
        printf("<!-- DEBUG: exec(%s) -->\n", command);
        execve("/bin/sh", argv, environ);
	log_it("web/email", "Validation key sent to %s at %s.", user->username, user->RGemail);
        exit(127);
    }
    return 0;
}

user_t *
newuser_makesupp()
{

    int a;
    long aa;
    user_t *user;

    user = (user_t *) xmalloc(sizeof(user_t));

    /* write zeros to a byte string */
    memset(user, 0, sizeof(user_t));

    strcpy(user->username, "newbie");

    for (a = 0; a < MAXQUADS; a++) {
	user->lastseen[a] = 0L;
	user->generation[a] = -1;
	user->forget[a] = -1;
    }
    user->flags = (US_BEEP | US_NOHIDE | US_PAUSE | US_NOTIFY_FR);
    user->priv = 0;
    user->screenlength = 24;

    for (a = 0; a < 5; a++)
	user->RA_rooms[a] = -1;

    strcpy(user->lasthost, "\1rnewbie.com");
    strcpy(user->doing, "\1R\1yI'm a new species!");

    time(&aa);
    user->laston_from = aa;
    user->laston_to = aa;
    user->firstcall = aa;
    return user;
}

void
fill_out_fields(user_t * user)
{

    strcpy(user->username, username);
    set_password( user, password );
    strcpy(user->RGname, name);
    strcpy(user->RGaddr, street);
    strcpy(user->RGcity, city);
    strcpy(user->RGstate, state);
    strcpy(user->RGcountry, country);
    strcpy(user->RGzip, zipcode);
    strcpy(user->RGphone, phone);
    strcpy(user->RGemail, email);
    if (strlen(url) > 16)
	strcpy(user->RGurl, url);
    else
	strcpy(user->RGurl, "");


}

void
generate_new_key(user_t * user)
{

    time_t bing;

    time(&bing);
    srandom(bing);
    user->validation_key = random() % 9999;
}

void
format_fields(user_t * user)
{

    int i = 0, j = 0;

    user->username[0] = toupper(user->username[0]);

    for (i = 0; i < strlen(user->username); i++) {

	if (user->username[i] == ' ') {
	    j = i + 1;
	    if (user->username[j] == ' ')
		user->username[i] = '\0';
	    else
		user->username[j] = toupper(user->username[j]);
	}
    }

}

void
error_2(user_t * user)
{

    printf("<h3 align=\"center\">That user already exists</h3>\n");
    printf("<p align=\"left\">\n");
    printf("The user <b>%s</b> already exists, please choose another name.\n", user->username);
    printf("<p align=\"left\">\n");
    printf("Go <a href=\"%s\">back</a> to the registration form.\n", referer);
    return;

}

void
url_name(char *string)
{

    int i = 0;


    for (i = 0; i < strlen(string); i++) {
	if (string[i] == ' ')
	    string[i] = '+';
    }
    return;

}
/* eof */
