#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>


//////////////////////////////////////////
// Les Boutons
#define BT_MAX		4 // Nb de boutons
#define BT_TIME		500	// Temps en ms entre 2 check de boutons
// Un nom pr les reconnaitres
#define BT_LEFT 	0
#define BT_RIGHT	2
#define BT_UP		3
#define BT_DOWN		1
// #define BT_ALIM		12 // N° PIN
#define BT_SEUIL	1022 // Seuil du analogRead qui indiquera l'appuis sur un Bt. (683 pr du 3,3V mais trop de perturbations et 1023 pr du 5v)


/////////////////////////////////////////
// LCD
#define LCD_LINE_MAX		50 // Nombre maximum de ligne dans un menu
#define LCD_LINE_MAX_DRAW	4 // Nombre maximum de ligne affichable (6 ligne total - 2 pr le titre menu)
#define LCD_CHAR_MAX		14 // Nombre maximum de caractère affichable (sur une ligne)

//////////////////////////////////////////
// Les Menus  * Doit être rempli à la main à chaque insertion d'un nouveau menu
#define ACCUEIL		1
#define CONTRAST	2
#define LUMIERE		3
#define POWER		4
#define AFFICHAGE	5
#define IP			6
#define SCRIPTS		7
#define WIFI		8
#define BLUETOOTH	9
#define BUREAU		10
#define SRVWEB		11
#define RASPBUGGY	12


#define DEBUG		0 // Mode debug?



// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 lcd = Adafruit_PCD8544(7, 6, 5, 4, 3);
int iLigne = 1; 	// Ligne actuelle
int iMenu = ACCUEIL;		// Menu actuel
// int iLigneMax = 3;	// Ligne Max du menu actuel


int iContrast = 50;
int aBtPin[BT_MAX] = { 0, 1, 2, 3 }; 	// Tableau contenant les PIN(analogiques) des  boutons
boolean aBtValue[BT_MAX];					// Tableau contenant les valeurs des  boutons



void setup(){
	/////////////////////////
	// Init Serial
	Serial.begin(9600);

	//////////////////////
	// Init LCD
	lcd.begin();
	lcd.setContrast( iContrast );
}
  
  
void loop() {
	
	
	//////////////////////////
	// Gestion Affichage
	menu_display( iMenu );
	
	/////////////////////////////////
	// Gestion Bouton
	
	// Attente d'appuis sur un bouton
	delay(BT_TIME);
	boolean bPressed = false;
	while( !bPressed ){
		bPressed = bt_read();
	}
	
	// Traitement du bouton
	for( int i=0; i < BT_MAX; i++ ){
		if( aBtValue[i] ){
			switch( i ){
			case BT_LEFT :
				if( DEBUG )
					Serial.println( "Gauche" );
					
				leave_menu();
			break;
			case BT_RIGHT :
				if( DEBUG )
					Serial.println( "Droite" );
					
				enter_menu();
			break;
			case BT_UP :
				if( DEBUG )
					Serial.println( "Haut" );
					
				if( iLigne -1 >= 1 )
					iLigne--;
				else
					iLigne = get_nbitems_menu(iMenu);
			break;			
			case BT_DOWN :
				if( DEBUG )
					Serial.println( "Bas" );
				
				if( iLigne +1 <= get_nbitems_menu(iMenu) )
					iLigne++;
				else
					iLigne = 1;
			break;
			default:
				if( DEBUG ){
					Serial.print( "Touche Inconnue : " );
					Serial.println( i );
				}
			break;
			}
		}
	}

	//bt_reset();	
}





////////////////////////
// Boutons

/* Fait le tour des boutons et retourne si un bouton a été appuyé.
 * Met à jours le tableau (var globale) des valeurs des boutons.
 */
boolean bt_read(){
	boolean bPushed = false;
	for( int i=0; i < BT_MAX; i++ ){
		if( analogRead( aBtPin[i] ) > BT_SEUIL ){
			bPushed = true;
			aBtValue[i] = true;
		}else
			aBtValue[i] = false;
	}
	return bPushed;
}

  
  
  
  
  
////////////////////////
// Template LCD/Menu

/* Efface l'écran, remet la police normale, et le curseur à 0,0
 */
void lcd_reset(){
	lcd.clearDisplay();
	lcd.setTextSize(1);
	lcd.setTextColor(BLACK);
	lcd.setCursor(0,0);	
}
/* Génère un "Bandeau" avec le texte fourni + Un eventuel texte apres.
 */
void menu_gen_titre( char * sTitre, char * sPrez){
	lcd.print( sTitre );
	lcd.print("--------------");	
	lcd.print(sPrez);	
}
/* Génère le corps d'un menu.
 * aItems_Menu : tableau avec les différents items de ce menu
 * iNbItems : Le nombre d'items dans ce menu
 * iPos : La ligne du curseur (de 1 à iNbItems)
 */
// void menu_gen_corps_dyn( char aItems_Menu[LCD_LINE_MAX][LCD_CHAR_MAX], int iNbItems, int iPos ){
void menu_gen_corps_dyn( int iNbItems, int iPos ,char ** aItems_Menu ){
	// S'il y a trop d'items dans ce menu, nous n'affichons que les 4 autour de la selection.
	int iStart = 0;
	int iStop = iNbItems;
	if( iNbItems > LCD_LINE_MAX_DRAW ){
		// Cas spécial du début.
		if( iPos <= 2 )
			iStart = 0;
		// Cas spécial de la fin
		else if( iPos > iNbItems -2 )
			iStart = iNbItems -4;
		// dans les autres cas : Une ligne apparait avant la selection, et 2 lignes apres.
		else
			iStart = iPos -2;
		
		// Le Stop sera toujours X ligne apres le start ;)
		iStop = iStart +LCD_LINE_MAX_DRAW;
			
	}
	
	if( DEBUG ){
		Serial.print( "Start : " );
		Serial.print( iStart );
		Serial.print( " Stop : " );
		Serial.println( iStop );
	}
	// Pour chacune des lignes 
	for( int i=iStart; i < iStop; i++ ){
		char sLigne[LCD_CHAR_MAX+1];
		int iTaille = strlen( aItems_Menu[i]);

		sLigne[LCD_CHAR_MAX] = '\0';
		// si on a selectionné cette ligne
		if( iPos -1 == i ){  // -1 pour le décalage ligne/tableau
			// On ajoute le symbole de debut
			strcpy( sLigne, ">" );
			// Puis le texte
			strcat( sLigne, aItems_Menu[i] );
			// On ajoute des espaces
			for( int j=iTaille; j<LCD_CHAR_MAX-1; j++ ){
				strcat( sLigne, " " );
			}
			
			// On inverse la couleur de cette ligne
			lcd.setTextColor(WHITE, BLACK); 

		}
		else{
			if( strcmp( aItems_Menu[i], "" ) == 0 )
				strcpy( sLigne, " " );
			else // On ajoute le symbole de debut
				strcpy( sLigne, "-" );
			// Puis le texte
			strcat( sLigne, aItems_Menu[i] );			
			// On ajoute des espaces
			for( int j=iTaille; j<LCD_CHAR_MAX-1; j++ ){
				strcat( sLigne, " " );
			}
			
			// Couleur normale
			lcd.setTextColor(BLACK); 
		}
		
		if( DEBUG )	
			Serial.println( sLigne )  ;
			
		lcd.print(sLigne);
	}
}



 

////////////////////////////
// Spécification des Menus
// * Chaque Fonction Doit être remplie à la main à chaque insertion d'un nouveau menu

/* Affiche le menu dont le numéro est fourni en paramètre.
 * Doit être rempli à la main à chaque insertion d'un nouveau menu
 */
void menu_display( int iNumMenu ){

	lcd_reset();
	switch( iNumMenu ){
	case ACCUEIL :
		menu_gen_ACCUEIL();
	break;
	case AFFICHAGE :
		menu_gen_AFFICHAGE();
	break;
	case CONTRAST :
		menu_gen_CONTRAST();
	break;
	case LUMIERE :
		menu_gen_LUMIERE();
	break;
	case POWER :
		menu_gen_POWER();
	break;
	case SCRIPTS :
		menu_gen_SCRIPTS();
	break;
	case WIFI :
		menu_gen_WIFI();
	break;
	case BLUETOOTH :
		menu_gen_BLUETOOTH();
	break;
	case BUREAU :
		menu_gen_BUREAU();
	break;
	case SRVWEB :
		menu_gen_SRVWEB();
	break;
	case RASPBUGGY :
		menu_gen_RASPBUGGY();
	break;
	default :
		menu_gen_ACCUEIL();
	break;
	}	
	lcd.display();

}
/* Retourne le nombre d'items contenu dans le menu fourni en paramètre
 * Doit être rempli à la main à chaque insertion d'un nouveau menu
 */
int get_nbitems_menu( int iNumMenu ){
	switch( iNumMenu ){
	case ACCUEIL :
		return 4;
	break;
	case AFFICHAGE :
		return 2;
	break;
	case CONTRAST :
		return 3;
	break;
	case LUMIERE :
		return 2;
	break;
	case POWER :
		return 5;
	break;
	case SCRIPTS :
		return 5;
	break;
	case WIFI :
		return 3;
	break;
	case BLUETOOTH :
		return 2;
	break;
	case BUREAU :
		return 4;
	break;
	case SRVWEB :
		return 2;
	break;
	case RASPBUGGY :
		return 2;
	break;
	default :
		return 0;
	break;
	}
}
/* Details de chacun des Menus.
 */
void menu_gen_ACCUEIL(){
	menu_gen_titre( "RaspTools Menu", "" );
	
	char * aTexts[] = { "Affichage", "IP", "Scripts", "Powers" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne

	const int iNbItems = get_nbitems_menu( iMenu /* ACCUEIL */ );
	menu_gen_corps_dyn( iNbItems, iLigne, aTexts );
	
} 
void menu_gen_AFFICHAGE(){
	menu_gen_titre( "-=Affichage=- ", "" );
	
	char * aTexts[] = { "Contrast", "Lumiere"}; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne

	const int iNbItems = get_nbitems_menu( iMenu /* ACCUEIL */ );
	menu_gen_corps_dyn( iNbItems, iLigne, aTexts );
	
} 
void menu_gen_CONTRAST(){
	menu_gen_titre( "-= Contrast =-", "" );
	
	//////////////
	// Gestion du contraste
	if( iLigne == 1 )
		iContrast += 2;
	else if( iLigne == 3 )
		iContrast -= 2;
	lcd.setContrast( iContrast );

	char sContrast[4] = {0};
	itoa( iContrast, sContrast, 10);
	// 3 items, iLigne sera toujours sur 2, mais on saura si on a monté ou déscendu grace aux 2 items autour.
	char * aTexts[] = { "", sContrast, "" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne
	
	iLigne = 2;
	const int iNbItems = get_nbitems_menu( iMenu /* CONTRAST */ );
	menu_gen_corps_dyn( iNbItems, iLigne, aTexts );
	
} 
void menu_gen_LUMIERE(){
	menu_gen_titre( "-= Lumiere =- ", "" );
	
	char * aTexts[] = { "ON", "OFF" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne
	
	const int iNbItems = get_nbitems_menu( iMenu /* LUMIERE */ );
	menu_gen_corps_dyn( iNbItems, iLigne, aTexts );
	
} 
void menu_gen_POWER(){
	menu_gen_titre( "-=  Power  =- ", "" );
	
	char * aTexts[] = { "Halt PI", "Reboot PI", "Halt Arduino", "Start Screen", "Stop Screen" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne
	
	const int iNbItems = get_nbitems_menu( iMenu /* POWER */ );
	menu_gen_corps_dyn( iNbItems, iLigne, aTexts );
	
} 
void menu_gen_SCRIPTS(){
 	menu_gen_titre( "-= Scripts =- ", "" );
	
	char * aTexts[] = { "Bureau", "Web Server", "RaspBuggy", "WIFI", "BlueTooth" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne
	
	const int iNbItems = get_nbitems_menu( iMenu );
	menu_gen_corps_dyn( iNbItems, iLigne, aTexts );

} 
void menu_gen_WIFI(){
 	menu_gen_titre( "-=   WIFI   =-", "" );
	
	char * aTexts[] = { "Scan", "Crack", "Connect" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne
	
	const int iNbItems = get_nbitems_menu( iMenu );
	menu_gen_corps_dyn( iNbItems, iLigne, aTexts );

} 
void menu_gen_BLUETOOTH(){
 	menu_gen_titre( "-=BlueTooth=- ", "" );
	
	char * aTexts[] = { "Scan", "Connect" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne
	
	const int iNbItems = get_nbitems_menu( iMenu );
	menu_gen_corps_dyn( iNbItems, iLigne, aTexts );

} 
void menu_gen_BUREAU(){
 	menu_gen_titre( "-=  Bureau  =-", "" );
	
	char * aTexts[] = { "Start X", "Stop X", "Start PMAD", "Stop PMAD" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne
	
	const int iNbItems = get_nbitems_menu( iMenu  );
	menu_gen_corps_dyn( iNbItems, iLigne, aTexts );

} 
void menu_gen_SRVWEB(){
 	menu_gen_titre( "-=Web Server=-", "" );
	
	char * aTexts[] = { "Start Server", "Stop Server" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne
	
	const int iNbItems = get_nbitems_menu( iMenu );
	menu_gen_corps_dyn( iNbItems, iLigne, aTexts );

} 
void menu_gen_RASPBUGGY(){
 	menu_gen_titre( "-=RaspBuggy=- ", "" );
	
	char * aTexts[] = { "Mode Control", "Mode Auto" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne
	
	const int iNbItems = get_nbitems_menu( iMenu );
	menu_gen_corps_dyn( iNbItems, iLigne, aTexts );

} 

/* Action a suivre lorsqu'on appuis sur le bouton de droite.
 * se sert de la variable globale iMenu
 */
void enter_menu( ){
	switch( iMenu ){
	case ACCUEIL :
		switch( iLigne ){
		case 1 :
			iMenu = AFFICHAGE;
			iLigne = 1;
		break;
		case 2 :
			iMenu = IP;
			iLigne = 1;
		break;
		case 3 :
			iMenu = SCRIPTS;
			iLigne = 1;
		break;
		case 4 :
			iMenu = POWER;
			iLigne = 1;
		break;
		default: 
		break;
		}
	break;
	case AFFICHAGE :
		if( iLigne == 1 ){
			iMenu = CONTRAST;
			iLigne = 2;
		}else{
			iMenu = LUMIERE;
			iLigne = 1;
		}
	break;	
	case LUMIERE :
		if( iLigne == 1 )
			; // ON
		else
			; // OFF
	break;
	case POWER : 
		switch( iLigne ){
		case 1 :
			Serial.println( "shutdown -h now" ); // Halt
		break;
		case 2 :
			Serial.println( "shutdown -r now" ); // Reboot
		break;
		case 3 :
			Serial.println( "/chemin/gpio write PIN_ARDUINO 0" ); // Halt Arduino
		break;
		case 4 :
			Serial.println( "/chemin/gpio write PIN_SCREEN 1" ); // Start Screen
		break;
		case 5 :
			Serial.println( "/chemin/gpio write PIN_SCREEN 0" ); // Stop Screen
		break;
		default:
		break;
		}
	break;
 	case SCRIPTS :
		switch( iLigne ){
		case 1 :
			iMenu = BUREAU;
			iLigne = 1;
		break;
		case 2 :
			iMenu = SRVWEB;
			iLigne = 1;
		break;
		case 3 :
			iMenu = RASPBUGGY;
			iLigne = 1;
		break;
		case 4 :
			iMenu = WIFI;
			iLigne = 1;
		break;
		case 5 :
			iMenu = BLUETOOTH;
			iLigne = 1;
		break;
		default:
		break;
		}
	break;
	case BUREAU :
		switch( iLigne ){
		case 1 :
			Serial.println( "startx" );// Start X
		break;
		case 2 :
			Serial.println( "stopx" );// Stop X
		break;
		case 3 :
			Serial.println( "startPMAD" );// Start PMAD
		break;
		case 4 :
			Serial.println( "stopPMAD" );// Stop PMAD
		break;
		default:
		break;
		}
	break;
	case SRVWEB :
		switch( iLigne ){
		case 1 :
			Serial.println( "startwebserver" );// Start Srv
		break;
		case 2 :
			Serial.println( "stopwebserver" );// Stop Srv
		break;
		default:
		break;
		}
	break;
	case RASPBUGGY :
		switch( iLigne ){
		case 1 :
			Serial.println( "startRaspbuggyControl" );// Mode Control
		break;
		case 2 :
			Serial.println( "startRaspbuggyAuto" );// Mode Auto
		break;
		default:
		break;
		}
	break;
	case WIFI :
		switch( iLigne ){
		case 1 :
			Serial.println( "WifiScan" );// Scan
		break;
		case 2 :
			Serial.println( "WifiCrack" );// Crack
		break;
		case 3 :
			Serial.println( "WifiConnect" );// Connect
		break;
		default:
		break;
		}
	break;
	case BLUETOOTH :
		switch( iLigne ){
		case 1 :
			Serial.println( "BT Scan" );// Scan
		break;
		case 2 :
			Serial.println( "BT Connect" );// Connect
		break;
		default:
		break;
		}
	break;
	default:
		if( DEBUG )
			Serial.println( "Menu inconnu" );
	break;
	}
}

/* Action a suivre lorsqu'on appuis sur le bouton de gauche.
 * se sert de la variable globale iMenu
 */
void leave_menu( ){
	switch( iMenu ){
/* 	case ACCUEIL : 	// --> pas necessaire de déclarer tout les menus...! par defaut : on retourne sur l'accueil.
					// Seul inconvénient ... on revient tjs sur la 1ere ligne ... alors qu'on avait selectionné la 2eme, 3eme....*/
	case CONTRAST :
	case LUMIERE :
		iMenu = AFFICHAGE;
		iLigne = 1;
	break;
 	case BUREAU :
	case SRVWEB :
	case RASPBUGGY :
	case WIFI :
	case BLUETOOTH :
		iMenu = SCRIPTS;
		iLigne = 1;
	break; 
	default:
		iMenu = ACCUEIL;
		iLigne = 1;
		if( DEBUG )
			Serial.println( "Menu inconnu" );
	break;
	}
}