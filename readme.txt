Pitur Sebastian 324CA

subscriber.c 

Am deschis un socket tcp pentru comunicarea cu serverul pe care am dezactivat 
algoritmul lui Neagle. Dupa ce am completat structura de socket cu datele 
serverului, am trimis cererea de conexiune prin "connect".

Daca cererea de conectare e acceptata, astept sa primesc un id request de la
server adica structura "server_to_client_hdr" cu type = 5, ca raspuns trimit o 
structura "client_to_server_hdr" de tipul 2 adica id reply.

Adaug in setul de socketi, socketul tcp pentru comunicarea cu serverul si STDIN.
Multiplexez intre acesti 2 socketi, iar daca primesc o comanda de la stdin, o parsez
cu "sscanf" si apelez functia corespunzatoare.

Pentru subscribe, creez o structura de comunicare de la client la server de tipul 0,
care ii va spune serverului ca este vorba de un subscribe, apoi astept confirmare ca 
s-a procesat comanda. Datele efectiv ramase dupa extragerea header-ului sunt topicul
si sf-ul.

Pentru unsubscribe, identic cu subscribe, numai ca se foloseste o structura cu tipul 1
care corespunde unei comenzi de tip unsubscribe.

Daca se primeste comanda exit, atunci se trimite o structura cu tipul 3, pe care 
serverul o sa o interpreteze ca cerere de deconectare, astept sa primesc confirmare
apoi inchid socket-ul.

In cazul in care se primeste un pachet de la server, extrag header-ul corespunzator 
"server_to_client_hdr" si apoi extrag datele in functia 
"extract_server_to_client_header", unde in functie de cati bytes de date s-au trimis
si de tipul de date trimis, afisez un mesaj la stdout.

Daca se primeste un mesaj de tipul 4 adica "Connection refused" se iese din loop-ul 
infinit si se inchide socket-ul.

server.c 

In server, aloc o lista simplu inlantuita in care o sa tin structuri de tip 
"client_structure", care retine id-ul, topicurile la care e abonat intr-o alta lista
coada de pachete ce s-ar fi trimis daca nu era offline, socket-ul care este -1 cat 
timp este offline si un camp denumit "online" care este 0 sau 1 pe parcursul 
programului.

Deschid socket-ul tcp pasiv pe care astept cereri de conexiune de la clienti, 
dupa care fac bind si listen pe el, dupa ce am completat structura de socket cu
datele server-ului.

Deschid socket-ul udp pe care astept mesaje de la clientul udp, care urmeaza sa 
fie redirectionate la clientii tcp. Initial, in setul de file descriptori se afla
cel de stdin, socketul pasiv tcp si socketul udp.

Multiplexez intre acestia, iar daca se incearca comunicarea pe socket-ul pasiv tcp, 
inseamna ca cineva doreste sa se conecteze. In functia "manage_connection_request"
accept cererea, atribuindu-i un nou socket "new_sockfd", intializez structura client,
trimit un id request, astept id reply, verific daca mai exista client cu acelasi id.
Daca nu, adaug in container-ul de clienti si in setul de file descriptori, afisand
un mesaj corespunzator la stdout. In caz contrar, daca un client online are acelasi id
ii trimit un mesaj de tipul "Connection refused", tipul 4 in structura de la server la
client, daca nu este deja online, se goleste coada de mesaje ce trebuiau trimise si
se adauga in setul cu file descriptori noul socket.

Daca primesc un mesaj udp, construiesc o structura message in care salvez topicul, 
tipul de date si valoare efectiva in functie de ce tip este, cu ajutorul functiei
"build_msg". Apoi cu build_sentBuffer, construiesc mesajul de trimis spre clientul
tcp, extragand din structura completata prin "recv_from" ip-ul clientului udp si 
portul acestuia, completand cu ele header-ul server_to_client_hdr, ca mai apoi in 
functia send_data sa trimit in functie de topicul mesajului tuturor clientilor care
il au in abonari, iar daca nu sunt online si flagul sf este setat sa il pun in coada
fiecaruia. Daca este online, astept confirmare ca s-a primit adica structura de la 
client la server de tipul 4.

Daca primesc comanda exit la stdin, cu functia "close_all_sockets", trimit mesaj de tip
"connection refused"(tipul 4) si astept confirmarea ca clientul l-a primit dupa care inchid
socket-ul si trec la urmatorul.

Daca primesc o comanda de la un client deja conectat in functie de valoare type din 
header-ul "client_to_server_hdr", daca este subscribe, caut clintul dupa socket si adaug
in lista lui de abonari, daca este deja abonat schimb sf-ul. Daca e vorba de unsubscribe
sterg din lista lui de abonari topicul repectiv, iar daca  e vorba de "exit", returnez 
socket-ul ca mai apoi dupa ce trimit confirmarea ca s-a procesat comanda sa il inchid. Daca
nu se intra pe ramura cu exit se returneaza -1, si nu se inchide niciun socket deja activ.
La final, inchid socket-urile tcp si udp initiale, daca se iese din loop-ul infinit prin 
comanda exit.
