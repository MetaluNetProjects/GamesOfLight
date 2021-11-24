# Puissance4 Pd

## Contenu : 

- puissance4pd.pd : le patch qui détecte les attaques sur les entrées 
                    micro 1 et 2 et qui renvoie l'info sur le port OSC.8185

- testOSCreceive.pd : un patch qui sert à tester le précedent; il contient 
                      juste un récepteur OSC, configuré sur la même adresse 8185
                      



## Installation :

Installer PureData (voir par exemple : http://puredata.info/downloads).
La version n'est pas importante. (disons au moins 0.38).

Lancer Pd, ouvrir le patch puissance4pd.pd.
Il est possible de lancer Pd sans l'affichage graphique :

[chemin de Pd]/pd -nogui -open [chemin de puissance4pd]/puissance4pd.pd



## Configuration :

Sélectionner la bonne carte son dans Pd, par le menu Fichier/Préférences/Paramètres audio, 
puis cliquer sur "Enregistrer tous les réglages".

Régler les gains des entrées micro sur les cartes son, afin d'obtenir la sensibilité voulue.
