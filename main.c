// Fichiers d'entêtes
#include <stdio.h>
#include "barriere.h"

// Définition des états de la machine à état
typedef enum{INIT = 0, TRAIN_ENTRANT = 1, TRAIN_MILIEU = 2, OUVERTURE_BARRIERE = 3, TRAIN_SORTANT = 4}tEtat;

// Définition des états des sorties si nécessaire
typedef enum{BARRIERE_OUVERTE = 0, BARRIERE_FERMEE = 1}tEtatBarriere;

/**@brief Fonction permettant le calcul de l'état actuel grâce aux trois capteurs.
 * Si l'état du train est INIT et que le capteur gauche ou droit est à 0, alors l'état passe à TRAIN_ENTANT.
 * Si l'état du train est TRAIN_ENTRANT et que le capteur central est à 0, alors l'état passe à TRAIN_MILIEU.
 * Si l'état du train est TRAIN_MILIEU et que le capteur central est à 1, alors l'état passe à OUVERTURE_BARRIRE.
 * Si l'état du train est OUVERTURE_BARRIERE et que le capteur gauche ou droit est à 0, alors l'état passe à TRAIN_SORTANT.
 * Si l'état du train est TRAIN_SORTANT et que le capteur gauche et le capteur droit est à 1, alors l'état passe à INIT.
 *  */ 
/// @param dpD 
/// @param dpG 
/// @param dpC 
/// @param etat 
void Transition_FSM(int dpD, int dpG, int dpC, tEtat *etat);

/** @brief Fonction permettant le calcul de l'état de la barrière grâce à l'état du train (position du train).
 * Elle permet de savoir si la barrière est ouverte ou fermée.
 * Ici elle est ouverte si le train à pour état : INIT, OUVERTURE_BARRIERE ou TRAIN_SORTANT.
 * Elle est fermée si le train à pour état : TRAIN_ENTRANT, TRAIN_MILIEU.
 * 
// @param etat 
// @param etat_barriere */ 
void Sorties_FSM(tEtat *etat, tEtatBarriere *etat_barriere);

/** @brief Fonction affichant dans la console la position du train et l'état de la barrière en temps réel
 * Si l'état du train est TRAIN_ENTRANT la console affiche : "Train entrant. \n -> Barrière fermée".
 * Si l'état du train est TRAIN_MILIEU la console affiche : "Le train a atteint la zone du milieu.".
 * Si l'état du train est OUVERTURE_bARRIERE la console affiche : "-> Ouverture de la barrière.".
 * Si l'état du train est TRAIN_SORTANT la console affiche : "Train sortant..
 * */ 
/// @param etat 
/// @param etat_precedent 
void Afficher_Etat_FSM(tEtat *etat, tEtat etat_precedent);

/** @brief Fonction de test de la fonction Transition_FSM().
 * Cette fonction demande des paramètres précis avant de les faire passer dans fonction Transition_FSM() pour prévoir une situation.
 * Si le résultat renvoyé par la fonction Transition_FSM() est le même que l'état prévu initialement lors de l'appel de la fonction, le test est réussi.
 * Une fois le test effectué un message comparant les deux états est affiché dans la console.
 * */ 
/// @param etat 
/// @param dpG 
/// @param dpC 
/// @param dpD 
/// @param etat_attendu 
/// @return 
int Test_Transition_FSM(tEtat etat, int dpG, int dpC, int dpD, tEtat etat_attendu);

/** @brief Fonction de test de la fonction Sorties_FSM().
 * Cette fonction demande des paramètres précis avant de les faire passer dans fonction Sorties_FSM() pour prévoir une situation.
 * Si le résultat renvoyé par la fonction Sorties_FSM() est le même que l'état prévu initialement lors de l'appel de la fonction, le test est réussi.
 * Une fois le test effectué un message comparant les deux états est affiché dans la console.
 * */ 
/// @param etat 
/// @param etat_barriere_attendu 
/// @return 
int Test_Sorties_FSM(tEtat etat, tEtatBarriere etat_barriere_attendu);

/**
 * @brief Fonction principale du programme
 * 
 * @return int 0 si succès
 */
int main()
{
    // Variables d'état des détecteurs
    int dpG = 1, dpC = 1, dpD = 1; // Pas de présence de train par défaut
    tEtat etat = INIT, etat_precedent = INIT;

    // Définir vos variables ici
    tEtatBarriere etat_barriere = BARRIERE_OUVERTE;

    // Initialisation de la communication avec l'interface de la barrière.
    // Remplacer /dev/ttyACM0 par ce qui convient si nécessaire.
    // Taper ls /dev/ttyACM* dans un terminal pour connaitre les numéros des ports existants
    // lorsque la carte est déconnectée puis carte connectée pour connaitre le numéro
    // du port à utiliser.
    int statut = initCommunicationBarriere("/dev/ttyACM0");
    if (statut != 0) {
        printf("Erreur d'initialisation de la communication avec la barrière. Changer le numéro du port !\n\r");
        return -1;
    }
    // Début du code
    printf("Automatisation de la barrière de passage à niveau\n\r");

    //Test Unitaires
    if(Test_Transition_FSM(INIT, 0, 0, 0, TRAIN_ENTRANT)!=1)printf("erreur lors du test de la fonction Transition_FSM()");
    if(Test_Sorties_FSM(TRAIN_MILIEU, BARRIERE_FERMEE)!=1)printf("erreur lors du test de la fonction Sorties_FSM()");

    // Boucle principale du programme
    while(1) {
        // Lecture des données d'entrées en utilisant la fonction litEtatDetecteurs
        litEtatDetecteurs(&dpG, &dpC, &dpD);

        // Calcul de l'état de la barrière en fonction des entrées
        etat = etat_precedent;
        Transition_FSM(dpD, dpG, dpC, &etat); 

        // Calcul des sorties en fonction de l'état de la barrière
        Sorties_FSM(&etat, &etat_barriere);

        // Ecriture de l'état de la barrière avec la fonction ecritEtatBarriere
        ecritEtatBarriere(etat_barriere);
       
        // Affichage de l'état du système
        Afficher_Etat_FSM(&etat, etat_precedent);
        
    }
    // Fermeture de la liaison série (aucune utilité car boucle infinie au dessus...)
    fermeCommunicationBarriere();
    return 0; // Succès
};

void Transition_FSM(int dpD, int dpG, int dpC, tEtat *etat){
    switch(*etat){
        case INIT:
            if(dpG == 0 || dpD == 0){
                *etat = TRAIN_ENTRANT;
            }
            break;
        case TRAIN_ENTRANT:
            if(dpC == 0){
                *etat = TRAIN_MILIEU;
            }
            break;
        case TRAIN_MILIEU:
            if(dpC == 1){
                *etat = OUVERTURE_BARRIERE;
            }
            break;
        case OUVERTURE_BARRIERE:
            if(dpG == 0 || dpD == 0){
                *etat = TRAIN_SORTANT;
            } 
            break;
        case TRAIN_SORTANT:
            if(dpG == 1 && dpD == 1){
                *etat = INIT;
            }
            break;
        default:
            *etat = INIT;
            break;
    }
}

void Sorties_FSM(tEtat *etat, tEtatBarriere *etat_barriere){

    switch(*etat){
            case INIT:
                *etat_barriere = BARRIERE_OUVERTE;
                break;
            case TRAIN_ENTRANT:
                *etat_barriere = BARRIERE_FERMEE;
                break;
            case TRAIN_MILIEU:
                *etat_barriere = BARRIERE_FERMEE;
                break;
            case OUVERTURE_BARRIERE:
                *etat_barriere = BARRIERE_OUVERTE;
                break;
            case TRAIN_SORTANT:
                *etat_barriere = BARRIERE_OUVERTE;
                break;
            default:
                *etat_barriere = BARRIERE_FERMEE;
                break;
        }
    
}


void Afficher_Etat_FSM(tEtat *etat, tEtat etat_precedent){

    if(*etat != etat_precedent){
            if(*etat == TRAIN_ENTRANT){
            printf("Train entrant.\n\r");
            printf("-> Barrière fermée.\n\r");
            }
            if(*etat = TRAIN_MILIEU){
                printf("Le train a atteint la zone du milieu.\n\r");
            }
            if(*etat = OUVERTURE_BARRIERE){
                printf("-> Ouverture de la barrière.\n\r");
            }
            if(*etat = TRAIN_SORTANT){
                printf("Train sortant.\n\r");
                printf("\n--------\n\n");
            }
            
        }
}


int Test_Transition_FSM(tEtat etat, int dpG, int dpC, int dpD, tEtat etat_attendu){

    int resultat = 0;

    Transition_FSM(dpD, dpC, dpG, &etat);
    if(etat == etat_attendu) resultat = 1;
    printf("\nFonction de test des transitions -> L'état actuel est %d et l'état attendu était %d.\n", etat, etat_attendu);

    return resultat;
}

int Test_Sorties_FSM(tEtat etat, tEtatBarriere etat_barriere_attendu){

    int resultat = 0;
    tEtatBarriere nouvel_etat_barriere;

    Sorties_FSM(&etat, &nouvel_etat_barriere);
    if(nouvel_etat_barriere == etat_barriere_attendu) resultat = 1;
    printf("Fonction de test des sorties -> L'état actuel est %d et l'état attendu était %d.\n\n", nouvel_etat_barriere, etat_barriere_attendu);

    return resultat;
}
