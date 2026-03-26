Am inceput rezolvarea pe baza laboratorului 9, rezolvat pe teams.
Am inceput rezolvarea de la propria mea tema facuta anul trecut!
Functii: - cookie_func: extrage cookie-urile din raspunsul http. La inceput am luat headerul Set-Cookie si am avansat pointerul dupa el.
                            Am gasit sfarsitul cookie-ului, apoi am calculat lungimea sa. Am alocat memori pentru el si l-am copiat.
         - token_func: extrage token-ul din raspunsul http. Am parsat raspunsul json, apoi am extras token-ul.
                            Dupa am alocat memorie pentru token ca string si l-am copiat.
Main: - cat timp aveam comenzi le citeam si verificam ce fel de comanda e
      - pentru comanda login_admin: Am deschis o conexiune la server si am citit username-ul si parola. Apoi, le-am adaugat in json si
            am convertit jsonul in string. Am alocat memorie pentru pachet si l-am copiat. Dupa, am facut cererea si am trimis-o serverului.
            In cele din urma, primeam raspuns de la server si afisam ce trebuie in functie de acesta si cand era cazul extrageam cookie-ul din
            raspuns. Daca eram in caz afirmativ faceam o variabila admin true pentru a stii ca sunt logata ca admin.
      - pentru comanda add_user: Prima data am verificat daca sunt conectat ca admin. Apoi, am deschis o conexiune la server si am citit username-ul si parola si le-am adaugat in json. Apoi,
            am verificat daca sunt valide si in functie de acesta afisam eroare sau nu. Dupa, am convertit json-ul in string. La final, am facut cererea si am trimis-o serverului
            si in functie de raspuns afisam ce trebuia.
      - pentru comanda get_users: Am verificat daca am drept de admin. Apoi, am deschis o conexiune la server si am creat o cerere. Am trimis-o si am primit un raspuns pentru ea.
            In functie de acel raspuns am afisat ce trebuia.
      - pentru comanda delete_user: Am verificat daca am drept de admin. Apoi, am deschis o conexiune la server si citesc usernameul verificand daca e corect. Daca da, creez ruta
            de acces pentru cerere, creez cererea si o trimit. In functie de raspuns afisez ce trebuie.
      - pentru comanda login: Am deschis o conexiune la server si am citit username-ul adminului, username-ul si parola. Apoi, le-am adaugat in json si
            am convertit jsonul in string. Am alocat memorie pentru pachet si l-am copiat. Dupa, am facut cererea si am trimis-o serverului.
            In cele din urma, primeam raspuns de la server si afisam ce trebuie in functie de acesta si cand era cazul extrageam cookie-ul din
            raspuns.
      - pentru comanda get_access: Am deschis o conexiune la server si am creat o cerere. Am trimis-o si am primit un raspuns pentru ea, tokenul.
            In functie de acel raspuns am afisat ce trebuia.
      - pentru comanda get_movies: Am deschis o conexiune la server si am creat o cerere. Am trimis-o si am primit un raspuns pentru ea.
            In functie de acel raspuns am afisat ce trebuia.
      - pentru comanda get_movie: Am citit id-ul filmului, iar daca acesta era gresit afisam eroare. Apoi, am deschis o conexiune la server
            si am creat o cerere. Am trimis-o si am primit un raspuns pentru ea. In functie de acel raspuns am afisat ce trebuia.
      - pentru comanda add_movie: Am deschis o conexiune la server si am citit titlul, anul, descrierea si ratingul si le-am adaugat in json. La an si rating,
            daca nu sunt corecte afisam eroare. Dupa, am convertit json-ul in string. La final, am facut cererea si am trimis-o serverului
            si in functie de raspuns afisam ce trebuia.
      - pentru comanda delete_movie: Am deschis o conexiune la server si citesc id-ul filmului verificand daca e corect. Daca da, creez ruta
            de acces pentru cerere, creez cererea si o trimit. In functie de raspuns afisez ce trebuie.
      - pentru comanda update_movie: Am deschis o conexiune la server si am citit titlul, anul, descrierea si ratingul si le-am adaugat in json. La an si rating,
            daca nu sunt corecte afisam eroare. Dupa, am convertit json-ul in string. La final, am facut cererea si am trimis-o serverului
            si in functie de raspuns afisam ce trebuia.
      - pentru comanda get_collections: Am deschis o conexiune la server si am creat o cerere. Am trimis-o si am primit un raspuns pentru ea.
            In functie de acel raspuns am afisat ce trebuia.
      - pentru comanda get_collection: Am citit id-ul colectiei, iar daca acesta era gresit afisam eroare. Apoi, am deschis o conexiune la server
            si am creat o cerere. Am trimis-o si am primit un raspuns pentru ea. In functie de acel raspuns am afisat ce trebuia.
      - pentru comanda add_collection: Am deschis o conexiune la server si am citit titlul si numarul filmelor si le-am adaugat in json. La numarul filmelor,
            daca nu e corect afisam eroare. Apoi, am citit id ul filmelor. Dupa, am convertit json-ul in string. La final, am facut cererea si am trimis-o serverului
            si in functie de raspuns afisam ce trebuia.
      - pentru comanda delete_collection: Am deschis o conexiune la server si citesc id-ul colectiei verificand daca e corect. Daca da, creez ruta
            de acces pentru cerere, creez cererea si o trimit. In functie de raspuns afisez ce trebuie.
      - pentru comanda add_movie_to_collection: Am deschis o conexiune la server si am citit id ul colectiei si al filmului si am adaugat in json id ul filmului dupa
            ce am verificat ca e corect. Dupa, am convertit json-ul in string. La final, am facut cererea si am trimis-o serverului
            si in functie de raspuns afisam ce trebuia.
      - pentru comanda delete_movie_to_collection: Am deschis o conexiune la server si am citit id ul colectiei si al filmului si am 
            verificat ca sunt corecte. Daca da, creez ruta de acces pentru cerere, creez cererea si o trimit. In functie de raspuns afisez ce trebuie.
      - pentru comanda logout_admin: Am deschis o conexiune la server si am creat o cerere. Am trimis-o si am primit un raspuns pentru ea.
            Daca raspunsul era afirmativ am facut adminul false pentru a stii ca nu mai sunt logata ca admin.
            In functie de acel raspuns am afisat ce trebuia si am resetat cookie-ul si token-ul.
      - pentru comanda logout: Am deschis o conexiune la server si am creat o cerere. Am trimis-o si am primit un raspuns pentru ea.
            In functie de acel raspuns am afisat ce trebuia si am resetat cookie-ul si token-ul.
      - pentru comanda exit: am iesit din while
