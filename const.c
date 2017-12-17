/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *			     Mud constants module			    *
 ****************************************************************************/  
   
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"
char *const npc_race[MAX_NPC_RACE] = 
   { "human", "elf", "dwarf", "hobbit", "pixie", "naga", "ogre", "orc", "troll", "half-elf", "gith", "drow", "sea-elf",
   "lizardman", "gnome", "demon", "sprite", "leprechaun", "halfling", "avian", "centaur", "slith", "feline", "undead",
   "harpy", "goblin", "kobold",
   "nymph", "troglodyte", "construct", "immortal", "green-dragon", "black-dragon", "white-dragon", "blue-dragon", "dragon",
   "gold-dragon", "silver-dragon",
   "bronze-dragon", "animal", "golem", "gorgon", "harpy", "hobgoblin", "kobold", "lizardman", "locust", "lycanthrope",
   "minotaur", "mold", "mule",
   "neanderthal", "ooze", "orc", "rat", "rustmonster", "shadow", "shapeshifter", "shrew", "shrieker", "skeleton", "slime",
   "snake", "spider", "stirge",
   "thoul", "troglodyte", "undead", "wight", "wolf", "worm", "zombie", "bovine", "canine", "feline", "porcine", "mammal",
   "rodent", "avis", "reptile",
   "amphibian", "fish", "crustacean", "insect", "spirit", "magical", "horse", "animal", "humanoid", "monster", "god" 
};
char *const lang_names[] = { "Kalorese", "Elven", "Dwarven", "Sunrise", "Orcish", "Dragon", "Islandic", "Drow" 
};
char *const moon_phase[PHASE_FULL + 1] = { "new", "crescent", "half", "gibbous", "full" };
char *const song_name[MAX_SONG] = 
   { "none", "peace", "silence", "sleep", "flight", "charm", "strength", "speed", "protection", "vigor", "fire", "thunder",
   "wind", "rivers", "sight", "shadows", "light", "winter", "life"
};
char *const talent_name[MAX_DEITY] = 
   { "Fire", "Earth", "Wind", "Frost", "Lightning", "Water", "Dream", "Speech", "Healing", "Death", "Change", "Time",
   "Motion", "Mind", "Illusion", "Seeking", "Security", "Catalysm", "Void" 
};
char *const talent_rank[MAX_DEITY][12] = { 
/* Fire */ 
   {"&RB&ru&Rrn&re&Rr", "&RF&ri&Rr&re&Rw&ra&Rlk&re&Rr", "&RFla&rmelo&Rver", "&RF&ri&Rr&re&Rst&ra&Rrt&re&Rr",
    "&RFl&ra&Rm&re&Rw&rea&Rv&re&Rr", "&RBl&ra&Rz&re&Rr", "&RI&rg&Rn&ri&Rt&re&Rr", "&RSc&ro&Rr&rc&Rh&re&Rr",
    "&RA&rr&Rs&ro&Rn&ri&Rst", "&rI&Rl&Yl&Wumina&Yt&Ro&rr", "&RP&ry&Rr&ro&Rm&ra&Rnc&re&Rr", "&RFl&ra&Rm&re&Rl&ro&Rrd"}, 
/* Earth */ 
   {"&GW&goo&Gd&gs&Gm&ga&Gn", "&GH&ge&Grb&ga&Gl&gi&Gst", "&GL&gea&Gfd&ga&Gnc&ge&Gr", "&GT&gr&Ge&ge&Gsp&gea&Gk&ge&Gr",
    "&wStonecaller", "&wMo&Ountaine&wer", "&GN&ga&Gt&gu&Gr&ga&Gl&gi&Gst", "&GR&ga&Gng&ge&Gr", "&GD&gr&Gu&gi&Gd",
    "&GG&geo&Gm&ga&Gnc&ge&Gr", "&GE&ga&Gr&gt&Gh W&ga&Grd&ge&Gn", "&GE&ga&Gr&gt&Gh K&gi&Gng"}, 
/* Wind */ 
   {"&CWin&cdlis&Ctener", "&CBr&cee&Cz&ce&Cm&ca&Ck&ce&Cr", "&CSk&cy&Cw&ca&Ct&cc&Ch&ce&Cr", "&CW&ci&Cn&cd&Csp&cea&Ck&ce&Cr",
    "&WClo&Cudcha&Wser", "&CG&ca&Cl&ce&Cc&ca&Cll&ce&Cr", "&CSt&co&Crm&cb&Cr&ci&Cng&ce&Cr", "&CW&ci&Cn&cd&Cw&ca&Clk&ce&Cr",
    "&CW&ci&Cn&cd&Cr&ci&Cd&ce&Cr", "&CZ&ce&Cph&cy&Crl&co&Crd", "&CT&ce&Cmp&ce&Cst", "&CSk&cy&Cl&co&Crd"}, 
/* Frost */ 
   {"&CC&Wo&Cl&Wd&Cb&Wl&Co&Wo&Cd", "&CIc&Wehea&Crt", "&CChi&Wll&Cer", "&WFr&Cee&Wz&Ce&Wr", "&WSnowmaker",
    "&WIce&Cc&Wall&Ce&Wr", "&WSl&Cee&Wt", "&WBlizzard", "&WWinter", "&WRi&Cm&Welo&Cr&Wd", "&WCry&Coman&Wcer",
    "&bIc&Bin&Cgd&Bea&bth"}, 
/* Lightning */ 
   {"&YZ&Wa&Ypp&We&Yr", "&YC&Wh&Ya&Wr&Yg&We&Yd", "&YSp&Wa&Yrk&We&Yr", "&YSh&Wo&Yck&We&Yr", "&YJ&Wolte&Yr",
    "&YS&Wi&Yzzl&We&Yr", "&YC&Wlou&Ydm&Wake&Yr", "&YElec&Wtri&Yfier", "&YSto&Wr&Ym&Wsee&Yker", "&zTh&Yu&znd&Ye&zr",
    "&YL&Wi&Yg&Wh&Yt&Wn&Yi&Wn&Yg", "&YE&Wl&Ye&Wc&Yt&Wr&Yo&Wm&Ya&Wn&Yc&We&Yr"}, 
/* Water */ 
   {"&BSw&bi&Bmm&be&Br", "&BW&ba&Bt&be&Br&bb&Br&bea&Bth&be&Br", "&BP&boo&Blsk&bi&Bmm&be&Br", "&BStr&bea&Bms&bi&Bng&be&Br",
    "&BR&bi&Bv&be&Brm&ba&Bg&be", "&BW&ba&Bt&be&Brd&ba&Bnc&be&Br", "&BW&ba&Bv&be&Br&bi&Bd&be&Br", "&BR&bai&Bnm&ba&Bk&be&Br",
    "&BS&bea&Bm&ba&Bg&be", "&BO&bc&Be&ba&Bn&bl&Bo&br&Bd", "&BH&by&Bdr&bo&Bm&ba&Bnc&be&Br", "&BTs&bu&Bn&ba&Bm&bi"}, 
/* Dream */ 
   {"&PSlee&Ypw&Palker", "&PDr&Yeam&Per", "&PSub&Yconsci&Pous", "&PImag&Yina&Ption", "&PDr&Yea&Pmw&Yatc&Pher",
    "&PDr&Yea&Pmw&Yalk&Per", "&zNightstalker", "&PDr&Yea&Pmsh&Yift&Per", "&PDr&Yea&Pm&Yak&Per", "&PDr&Yea&Pmw&Yeav&Per",
    "&PDr&Yea&Pml&Yo&Prd", "&zNightmare"}, 
/* Speech */ 
   {"&YSp&Oea&Yk&Oe&Yr", "&YW&Oai&Yl&Oe&Yr", "&pHy&Ppnoti&pst", "&YN&Oa&Ym&Oe&Yr", "&pH&Parpe&pr", "&pMu&Psici&pan",
    "&pC&Ponducto&pr", "&pS&Pong&pw&Prite&pr", "&pSpe&Pll&psi&Png&per", "&YS&Ou&Ymm&Oo&Yn&Oe&Yr", "&YC&Oa&Yll&Oe&Yr",
    "&PS&pi&Pr&pe&Pn"}, 
/* Healing */ 
   {"&WAcolyte", "&WCurative", "&WHe&Cal&Wer", "&WSh&Gam&Wan", "&WCleric", "&WPriest", "&WBen&Yevol&Went", "&WPurifier",
    "&WLifebringer", "&WLifegiver", "&WMiracle", "&WCreator"}, 
/* Death */ 
   {"&zTa&gint&zed", "&zCor&Orup&zted", "&zTa&gint&zer", "&zCorr&Oup&zter", "&zWa&prlo&zck", "&zSou&blkee&zper",
    "&zDea&rthbrin&zger", "&zSpi&critl&cord", "&zSou&rlrea&zver", "&zNecromancer", "&zPl&gag&zue", "&zGrim Reaper"}, 
/* Change */ 
   {"&YM&Oi&Ym&Oi&Yc", "&YSh&Oift&Yer", "&YSh&Oap&Yer", "&YSha&Opeshif&Yter", "&YWa&Orp&Yer", "&YD&Ooppe&Ylg&Oange&Yr",
    "&YCh&Oangeli&Yng", "&YMut&Oa&Yter", "&YCha&Omel&Yeon", "&YTr&Oansmut&Yer", "&YCh&Oang&Yer", "&YPo&Olymanc&Yer"}, 
/* Time */ 
   {"&PAu&pg&Pu&pr&Pe&pr", "&PFo&pre&Pte&pll&Per", "&BWa&brp&Ber", "&WBlur", "&CQu&ci&Cckl&ci&Cng",
    "&CH&ci&Cst&co&Cr&cia&Cn", "&CT&ci&Cm&ce&Cst&co&Cpp&ce&Cr", "&CT&ci&Cm&ce&Csh&ci&Cft&ce&Cr", "&PPr&po&Pph&pe&Pt",
    "&CT&ci&Cm&ce&Cl&co&Crd", "&CChr&co&Cn&co&Cm&ca&Cnc&ce&Cr", "&CE&ct&Ce&cr&Cn&ca&Cl"}, 
/* Motion */ 
   {"&YWa&Wlk&Yer", "&YWa&Wnder&Yer", "&YTe&Wleport&Yer", "&YDi&Wsplac&Yer", "&YKi&Wnet&Yic", "&YBl&Wink&Yer",
    "&YS&Wlide&Yr", "&YGa&Wtemak&Yer", "&YTra&Wnsloca&Ytor", "&YTr&Wavell&Yer", "&YIn&Wfini&Yte", "&YOm&Wniprese&Ynt"}, 
/* Mind */ 
   {"&PS&pen&Psi&pti&Pve", "&PE&pn&Pl&pi&Pg&ph&Pt&pe&Pn&pe&Pd", "&PEm&ppa&Pth", "&PM&pe&Pnt&pa&Pl&pi&Pst", "&PPs&pych&Pic",
    "&PMi&pndread&Per", "&PT&pe&Pl&pe&Pp&pa&Pt&ph", "&PTh&pou&Pghtl&pace", "&PM&pi&Pn&pd&Pb&pe&Pn&pd&Pe&pr",
    "&PPs&pio&Pn&pi&Pc&pi&Pst", "&PM&pi&Pn&pd &PF&pl&Pa&py&Pe&pr", "&PTh&pou&Pg&pht&Pl&po&Prd"}, 
/* Illusion */ 
   {"&RR&Ya&Gi&Cn&Bb&Po&pw", "&RP&Yr&Gi&Bs&Pm", "&wMistweaver", "&RLi&Ygh&Ctb&Ben&Pder", "&zShadowmaker", "&zConcealer",
    "&wUnseen", "&RH&Yo&Glo&Cg&Br&Pam", "&RC&Yh&Oa&Gm&Ce&Bl&be&Po&pn", "&zShadowlord", "&RIl&Ylu&Gsi&Con&Bis&Pt",
    "&RM&Yi&Gr&Ca&Bg&Pe"}, 
/* Seeking */ 
   {"&CW&ca&Ctch&ce&Cr", "&GF&gi&Gnd&ge&Gr", "&GH&gu&Gnt&ge&Gr", "&GL&go&Gc&ga&Gt&ge&Gr", "&GT&gr&Ga&gc&Gk&ge&Gr",
    "&CE&cx&Cpl&co&Cr&ce&Cr", "&GF&ga&Grs&gee&Gr", "&GCl&gai&Grv&go&Gy&ga&Gnt", "&GO&gr&Ga&gc&Gl&ge", "&GS&gee&Gk&ge&Gr",
    "&GA&gll-&GS&ge&Ge&gi&Gng", "&GO&gm&Gn&gi&Gsc&gie&Gnt"}, 
/* Security */ 
   {"&CG&cua&Crd", "&CB&ci&Cnd&ce&Cr", "&CH&co&Cld&ce&Cr", "&CSh&cie&Cld&ce&Cr", "&CB&ca&Crr&cie&Cr", "&CKn&ci&Cght",
    "&CWa&cllmak&Cer", "&CD&ce&Cf&ce&Cnd&ce&Cr", "&CPr&co&Ct&ce&Cct&co&Cr", "&CK&cee&Cp&ce&Cr", "&CW&ca&Crd&ce&Cr",
    "&CG&cua&Crd&cia&Cn"}, 
/* Catalysm */ 
   {"&PG&pi&Pft&pe&Pd", "&PCh&pa&Pnn&pe&Pl&pe&Pr", "&PC&pa&Pt&pa&Pl&py&Pst", "&PA&pl&Pch&pe&Pm&pi&Pst",
    "&PS&pc&Pr&pi&Pb&pe", "&PW&pi&Pz&pa&Prd", "&PE&pn&Pch&pa&Pnt&pe&Pr", "&PS&po&Prc&pe&Pr&pe&Pr",
    "&PSp&pe&Pl&pl&Pb&pi&Pnd&pe&Pr", "&PA&pr&Pc&ph&Pm&pa&Pg&pe", "&PA&pr&Pt&pi&Pf&pi&Pc&pe&Pr", "&PM&pa&Pn&pa&Pl&po&Prd"},
   
/* Void */ 
   {"&zShadeseer", "&zShadowwalker", "&WStarsinger", "&zVoi&bdwal&zker", "&zSpiritmaker", "&zVoi&bdrun&zner",
    "&zWe&Bav&zer", "&BWor&Cldsh&Baper", "&zRea&Blityl&zord", "&zDe&Bmig&zod", "&zIm&Bmort&zal", "&zOmn&Bipot&zent"} 
};
char *const weapon_brand[15] = 
   { "", "&Gpoisoned&W ", "&Rflaming&W ", "razor-sharp ", "&Cfrozen&W ", "&Yelectric&W ", "&gacidic&w ", "&cstormy&w ",
   "&Rr&Yad&Wi&Yan&Rt&W ", "&zcrushing&w ", "&zvampiric&w ", "&Pps&pio&Pn&pi&Pc&W ", "&zanti-matter&W ",
   "&Ys&Oo&Yn&Oi&Yc&W ", "&Btw&zist&Bed&W "
};
char *const weapon_bane[] = 
   { "", "undead", "demons", "elves", "orcs", "dragons", "humans", "dwarves", "fae", "giants", "minotaurs", "trolls",
   "halflings", "animals", "felines", "centaurs", "constructs", "angels" 
};
char *const part_locs[] = 
   { "head", "neck", "chest", "stomach", "back", "nose", "tail", "right hand", "left hand", "right arm", "left arm",
   "right leg", "left leg", "right foot", "left foot", "right eye", "left eye", "right wrist", "left wrist", "right ankle",
   "left ankle",
   "right hoof", "left hoof", "right paw", "left paw", "right wing", "left wing", "right ear", "left ear", "horn",
   "right horn", "left horn",
   "tongue", "fangs", "tentacles", "beak", "right foreleg", "left foreleg", "right rear leg", "left rear leg", "ass",
   "face", "right fin",
   "left fin", "dorsal fin", "penis", "vagina", "claws" 
};
char *const weapon_skill[20] = 
   { "Combat", "Swords", "Daggers and Knives", "Axes", "Whips and Flails", "Maces and Clubs", "Wands and Lasers",
   "Firearms", "Bows", "Crossbows", "Staves", "Halberds and Glaives", "Spears and Tridents", "Sickles and Scythes",
   "Talons and Claws",
   "Teeth and Fangs", "Horns and Headbutt", "Tail", "Hand to Hand", "Kicking"
};
char *const noncombat_skill[20] = 
   { "Smithing", "Alchemy", "Tailoring", "Stealth", "Streetwise", "Jewelry", "Nature", "Medicine", "Mining", "Channeling",
   "Linguistics", "(none)", "(none)", "(none)", "(none)", "(none)", "(none)", "(none)", "(none)", "(none)" 
};


/*
 * Liquid properties.
 * Used in #OBJECT section of area file.
 */ 
const struct liq_type liq_table[LIQ_MAX] = { {"water", "clear", {0, 1, 10}}, /*  0 */ 
{"beer", "amber", {3, 2, 5}}, 
   {"wine", "rose", {5, 2, 5}}, 
   {"ale", "brown", {2, 2, 5}}, {"gnomish ale", "pink", {5, 2, 5}}, {"whisky", "golden", {6, 1, 4}}, /*  5 */ 
{"lemonade", "yellow", {0, 1, 8}}, 
   {"firebreather", "boiling", {10, 0, 0}}, 
   {"urine", "yellow", {0, 3, 3}}, {"slime mold juice", "green", {0, 4, -8}}, {"milk", "white", {0, 3, 6}}, /* 10 */ 
{"tea", "tan", {1, 1, 6}}, 
   {"coffee", "black", {0, 1, 6}}, 
   {"blood", "red", {0, 2, -1}}, {"cider", "reddish-brown", {1, 1, 3}}, {"cola", "cherry", {0, 1, 5}}, /* 15 */ 
{"mead", "honey color", {4, 2, 5}}, /* 16 */ 
{"juice", "brown", {0, 2, 5}} /* 17 */ 
};
char *const magic_table[29] = 
   { "spell", "&Rfire&W", "&Wi&Cc&We", "&Ylightning&W", "&Penergy&w", "&Ga&gc&Gi&gd&w", "&gpoison&w", "&bdrain&w",
   "&Pth&pou&Pght&W", "&Wlight", "stones", "daggers", "swords", "&Cwind&W", "&Bwater&W", "&Gvines&W", "&Ys&Oou&Ynd&W",
   "&Pdr&Yea&Pm&W",
   "&Bdis&ztort&Bion&W", "&zantimatter&W", "&zantimagic&W", "&Wholy energy&w", "&zunholy energy&w", "&pf&Porc&pe&W",
   "&be&cth&ber&W",
   "&bdis&Btort&bion&W", "&Gp&Yrob&Ge&W", "&Rm&Ya&Gg&Bi&Pc&W" 
};
char *const pos_names[POS_DRAG + 1] = 
   { "dead", "wounded", "incapacitated", "unconscious", "sleeping", "channeling", "resting", "sitting", "kneeling",
   "squatting", "fighting", "swimming", "standing", "mounted", "flying", "shove", "drag" 
};


/* Weather constants - FB */ 
char *const temp_settings[MAX_CLIMATE] = { "cold", "cool", "normal", "warm", "hot", 
};
char *const precip_settings[MAX_CLIMATE] = { "arid", "dry", "normal", "damp", "wet", 
};
char *const wind_settings[MAX_CLIMATE] = { "still", "calm", "normal", "breezy", "windy", 
};
char *const mana_settings[MAX_CLIMATE] = { "dead", "stagnant", "stable", "charged", "energized", 
};
char *const preciptemp_msg[6][6] = { 
      /*
       * precip = 0 
       */ 
   {"Frigid temperatures settle over the land", "It is bitterly cold", "The weather is crisp and dry",
    "A comfortable warmth sets in", "A dry heat warms the land", "Seething heat bakes the land" }, 
      /*
       * precip = 1 
       */ 
   {"A few flurries drift from the high clouds", "Frozen drops of rain fall from the sky",
    "An occasional raindrop falls to the ground", "Mild drops of rain seep from the clouds",
    "It is very warm, and the sky is overcast", "High humidity intensifies the seering heat" }, 
      /*
       * precip = 2 
       */ 
   {"A brief snow squall dusts the earth", "A light flurry dusts the ground", "Light snow drifts down from the heavens",
    "A light drizzle mars an otherwise perfect day", "A few drops of rain fall to the warm ground",
    "A light rain falls through the sweltering sky" }, 
      /*
       * precip = 3 
       */ 
   {"Snowfall covers the frigid earth", "Light snow falls to the ground", "A brief shower moistens the crisp air",
    "A pleasant rain falls from the heavens", "The warm air is heavy with rain",
    "A refreshing shower eases the oppresive heat" }, 
      /*
       * precip = 4 
       */ 
   {"Sleet falls in sheets through the frosty air", "Snow falls quickly, piling upon the cold earth",
    "Rain pelts the ground on this crisp day", "Rain drums the ground rythmically", "A warm rain drums the ground loudly",
    "Tropical rain showers pelt the seering ground" }, 
      /*
       * precip = 5 
       */ 
   {"A downpour of frozen rain covers the land in ice", "A blizzard blankets everything in pristine white",
    "Torrents of rain fall from a cool sky", "A drenching downpour obscures the temperate day",
    "Warm rain pours from the sky", "A torrent of rain soaks the heated earth" } 
};
char *const windtemp_msg[6][6] = { 
      /*
       * wind = 0 
       */ 
   {"The frigid air is completely still", "A cold temperature hangs over the area", "The crisp air is eerily calm",
    "The warm air is still", "No wind makes the day uncomfortably warm", "The stagnant heat is sweltering" }, 
      /*
       * wind = 1 
       */ 
   {"A light breeze makes the frigid air seem colder", "A stirring of the air intensifies the cold",
    "A touch of wind makes the day cool", "It is a temperate day, with a slight breeze",
    "It is very warm, the air stirs slightly", "A faint breeze stirs the feverish air" }, 
      /*
       * wind = 2 
       */ 
   {"A breeze gives the frigid air bite", "A breeze swirls the cold air", "A lively breeze cools the area",
    "It is a temperate day, with a pleasant breeze", "Very warm breezes buffet the area",
    "A breeze ciculates the sweltering air" }, 
      /*
       * wind = 3 
       */ 
   {"Stiff gusts add cold to the frigid air", "The cold air is agitated by gusts of wind",
    "Wind blows in from the north, cooling the area", "Gusty winds mix the temperate air",
    "Brief gusts of wind punctuate the warm day", "Wind attempts to cut the sweltering heat" }, 
      /*
       * wind = 4 
       */ 
   {"The frigid air whirls in gusts of wind", "A strong, cold wind blows in from the north",
    "Strong wind makes the cool air nip", "It is a pleasant day, with gusty winds",
    "Warm, gusty winds move through the area", "Blustering winds punctuate the seering heat" }, 
      /*
       * wind = 5 
       */ 
   {"A frigid gale sets bones shivering", "Howling gusts of wind cut the cold air",
    "An angry wind whips the air into a frenzy", "Fierce winds tear through the tepid air",
    "Gale-like winds whip up the warm air", "Monsoon winds tear the feverish air" } 
};
char *const manatemp_msg[6][6] = { 
      /*
       * mana = 0 
       */ 
   {"The frozen landscape seems devoid of energy", "The surrounding terrain is cold and colorless",
    "The region seems stale and dead", "The warm area betrays the smell of lifelessness",
    "A foul stench filters through the heat", "The stench of death fills the sweltering surroundings" }, 
      /*
       * mana = 1 
       */ 
   {"This frozen location smells dry", "Life flows slowly in the icy surroundings",
    "A faint sparkle suggests the slightest hint of energy", "A trickle of energy dances in the area",
    "Tiny currents of energy ripple through this warm place", "The searing region breathes of life close to death" }, 
      /*
       * mana = 2 
       */ 
   {"Traces of energy fill this freezing place", "Small currents energize the frosty landscape",
    "This location murmurs with life", "A feeble spark dances in the mild surroundings",
    "A feeling of power settles softly over the region", "The sweltering heat radiates energy" }, 
      /*
       * mana = 3 
       */ 
   {"An occasional spark dances across the frozen surroundings", "This place shimmers faintly with a golden aura",
    "A faint beam of light shines from an indeterminate source", "A pleasant feeling fills the area",
    "A ripple of energy radiates from the hot surroundings", "Multicolored sparks drift down through the searing area" },
   
      /*
       * mana = 4 
       */ 
   {"Energy crackles through this frozen place", "Sparks wink briefly in your peripheral vision",
    "The landscape is teeming with small animal life",
    "A shimmering white aura surrounds everything for a moment, then vanishes",
    "A glow betrays the bound up energy in these lands", "Red lightning lights the sweltering landscape" }, 
      /*
       * mana = 5 
       */ 
   {"Torrents of pure mana pour from above", "Crackling blue lightning fills the icy landscape",
    "Multicolored auras dance about this place", "The surroundings all but drip with life essence",
    "Shimmering waves of pure life force dance on the horizon", "A torrent of pure energy engulfs the heated area" } 
};
char *const precip_msg[3] = 
   { "there is not a cloud in the sky", "pristine white clouds are in the sky", "thick, grey clouds mask the sky" 
};
char *const wind_msg[6] = 
   { "there is not a breath of wind in the air", "a slight breeze stirs the air", "a breeze wafts through the area",
   "brief gusts of wind punctuate the air", "angry gusts of wind blow", "howling winds whip the air into a frenzy" 
};
char *const clearday_msg[6] =
   { "The night sky is covered with stars.", "The early morning sky is pale grey as the sun rises.",
   "The late morning sky lights up as the sun reaches its peak.", "The sun shines brightly in the afternoon sky.",
   "The evening sky grows dim as the sun sets.", "The night sky is covered with stars." 
};
char *const cloudyday_msg[6] =
   { "The night sky is pitch black.", "The early morning sky is pale grey and shrouded in mist.",
   "The late morning sky is overcast and grey.", "The afteroon sky is covered with clouds.",
   "The evening sky is shadowed and dark.",
   "The night sky is pitch black." 
};


/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */ 
#define SLOT(n)	n
#define LI LEVEL_IMMORTAL
   
#undef AM 
#undef AC 
#undef AT 
#undef AW 
#undef AV 
#undef AD 
#undef AR
#undef AA
   
#undef LI
