#pragma once
#include <string_view>
#include <string>
#include <map>
#include <variant>
#include "frozenDictionary.hpp"

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
    ° frozen::set<int, 1..16>
    ° std::monostate (dans variant)

 */

// this map accessor wrapper verify that the key is valid @compile time
#define ConfigurationFile_AT(c,k)  ( \
    { \
      static_assert(conf_dict.find(k) != conf_dict.end());	\
      constexpr auto index = conf_dict.at(k).defaut.index();	\
      std::get<index>((c)[k]); })


using value_variant_t = std::variant<int, double, bool, std::string, std::monostate>;

class ConfigurationFile {
public:
  ConfigurationFile(const char* m_fileName) : fileName(m_fileName) {};
  bool populate(void);
  const value_variant_t& operator[] (const std::string_view key);
private:
  using dictionary_t = std::map<std::string, value_variant_t>;
  dictionary_t dictionary;
  const char *fileName;
  mutable MUTEX_DECL(mu);
  bool readConfFile(void);
  bool writeConfFile(void);
  bool verifyNotFilledParameters(void);
  void syslogInfoParameters(void);
};
