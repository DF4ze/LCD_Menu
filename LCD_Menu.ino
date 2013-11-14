
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
#define AFFICHAGE	2
#define CONTRAST	3
#define LUMIERE		4
#define POWER		5



const boolean bDebug = true;


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
				if( bDebug )
					Serial.println( "Gauche" );
					
				leave_menu();
			break;
			case BT_RIGHT :
				if( bDebug )
					Serial.println( "Droite" );
					
				enter_menu();
			break;
			case BT_UP :
				if( bDebug )
					Serial.println( "Haut" );
					 
				if( iLigne -1 >= 1 )
					iLigne--;
				else
					iLigne = get_nbitems_menu(iMenu);
			break;			
			case BT_DOWN :
				if( bDebug )
					Serial.println( "Bas" );
				
				if( iLigne +1 <= get_nbitems_menu(iMenu) )
					iLigne++;
				else
					iLigne = 1;
			break;
			default:
				if( bDebug ){
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
void menu_gen_corps_dyn( char aItems_Menu[LCD_LINE_MAX][LCD_CHAR_MAX], int iNbItems, int iPos ){
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
	
	if( bDebug ){
		Serial.print( "Start : " );
		Serial.print( iStart );
		Serial.print( " Stop : " );
		Serial.println( iStop );
	}
	// Pour chacune des lignes 
	for( int i=iStart; i < iStop; i++ ){
		char sLigne[LCD_CHAR_MAX+1] = {0};
		int iTaille = strlen( aItems_Menu[i]);

		// sLigne[LCD_CHAR_MAX] = '\0';
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

		}else{
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
		if( bDebug )
			Serial.println( sLigne );
			
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
		return 3;
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
	default :
		return 0;
	break;
	}
}
/* Details de chacun des Menus.
 */
void menu_gen_ACCUEIL(){
	menu_gen_titre( "RaspTools Menu", "" );
	
	char aTexts[LCD_LINE_MAX][LCD_CHAR_MAX] = { "Affichage", "Scripts", "Power" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne

	const int iNbItems = get_nbitems_menu( iMenu );
	menu_gen_corps_dyn( aTexts, iNbItems, iLigne );
	
} 
void menu_gen_AFFICHAGE(){
	menu_gen_titre( "-=Affichage=- ", "" );
	
	char aTexts[LCD_LINE_MAX][LCD_CHAR_MAX] = { "Contrast", "Lumiere" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne	

	const int iNbItems = get_nbitems_menu( iMenu );
	menu_gen_corps_dyn( aTexts, iNbItems, iLigne );
	
} 
void menu_gen_CONTRAST(){
	menu_gen_titre( "-= Contrast =-", "" );
	
	char sContrast[3];
	itoa( iContrast, sContrast, 10);
	sContrast[2] = '\0';
	char aTexts[LCD_LINE_MAX][LCD_CHAR_MAX] = { "", "", "" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne
	strcpy( aTexts[1], sContrast );
	
	//////////////
	// Gestion du contraste
	if( iLigne == 1 )
		iContrast += 2;
	else if( iLigne == 3 )
		iContrast -= 2;
	lcd.setContrast( iContrast );
	
	iLigne = 2;
	const int iNbItems = get_nbitems_menu( iMenu );
	menu_gen_corps_dyn( aTexts, iNbItems, iLigne );
	
} 
void menu_gen_LUMIERE(){
	menu_gen_titre( "-= Lumiere =- ", "              " );
	
	char aTexts[LCD_LINE_MAX][LCD_CHAR_MAX] = { "ON", "OFF" }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne

	const int iNbItems = get_nbitems_menu( iMenu );
	menu_gen_corps_dyn( aTexts, iNbItems, iLigne );
	
} 
void menu_gen_POWER(){
	menu_gen_titre( "-=  POWER  =- ", "" );
	
	char aTexts[LCD_LINE_MAX][LCD_CHAR_MAX] = { "Halt PI", "Reboot PI", "Halt Ard.", "Start Scr.", "Stop Scr." }; //!\\ Les textes doivent etre < LCD_CHAR_MAX --> (13) pour laisse la place au symbole de ligne

	const int iNbItems = get_nbitems_menu( iMenu );
	menu_gen_corps_dyn( aTexts, iNbItems, iLigne );
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
			// iMenu = SCRIPTS;
			// iLigne = 1;
			;
		break;
		case 3 :
			iMenu = POWER;
			iLigne = 1;
		break;
		default:
		break;
		}
	break;
	case AFFICHAGE :
		switch( iLigne ){
		case 1 :
			iMenu = CONTRAST;
			iLigne = 1;
		break;
		case 2 :
			iMenu = LUMIERE;
			iLigne = 1;
			;
		break;
		default:
		break;
		}
	break;
	default:
		if( bDebug )
			Serial.println( "enter_menu : Nous sommes dans un menu inconnu" );
	break;
	}
}

/* Action a suivre lorsqu'on appuis sur le bouton de gauche.
 * se sert de la variable globale iMenu
 */
void leave_menu( ){
	switch( iMenu ){
	case CONTRAST :
	case LUMIERE :
		iMenu = AFFICHAGE;
		iLigne = 1;	
	break;
	default:
		iMenu = ACCUEIL;
		iLigne = 1;
		if( bDebug )
			Serial.println( "leave_menu : Menu par default : ACCUEIL" );
	break;
	}
}