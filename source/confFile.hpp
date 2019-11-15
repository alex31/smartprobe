#pragma once

/*
  dependance : un log ouvert.


  confFile est juste pour :
  ° l'ouverture du fichier (init fatfs)
  ° lecture du fichier, analyse via regexp -> remplissage dictionnaire
  ° clef : string, valeur : variant<int, float, bool, std::string>
  ° l'accès au dictionnaire R/W
  ° l'écriture du dictionnaire dans un fichier
  
  mode opératoire :

  le fichier est lu et le dictionnaire disponible

  le contenu du dictionnaire est confronté à un autre dictionnaire en read only qui contient les 
  données metier :
  clef : string, valeur : struct<type, default value, bool, range>
  ° verification sur les types, sur le range
  ° si l'entrée n'existe pas, on la crée avec la valeur par defaut -> log info
  ° si l'entrée existe mais qu'il y a une erreur de type ou de range : log error puis halton continue
    de processer tout le fichier, mais à la fin on renvoie une condition d'erreur pour un halt après
    l'écriture du fichier généré : on remplace les entrées en erreur par une entrée avec la 
    valeur par defaut.
  ° si pas d'erreur : on enregistre un fichier actualConf.txt
  ° l'application travaille avec les valeurs du dictionnaire, eventuellement cachée, pour eviter
    la phase de lookup (en fonction de la criticité du temps d'accès)

    range pourait être un variant 
    ° std::pair(min max)
    ° etl::vector<int> (constexprisable ?) : liste de valeur autorisées
    ° struct no_tange_t {} : pas de range

 */


/*

  TODO : tester si etl::vector est constexprisableq

 */
