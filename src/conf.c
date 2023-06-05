/*
 *  Copyright (C) 2021 Steward Fu
 *  Copyright (C) 2001 Peponas Mathieu
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <getopt.h>
#include <zlib.h>
#include <SDL.h>

#include "unzip.h"
#include "conf.h"
#include "emu.h"
#include "memory.h"
#include "gnutil.h"

static struct {
  CONF_ITEM **conf;
  int size, nb_item;
} cf_hash[128] = {0};


void cf_cache_conf(void)
{
  conf.show_fps = CF_BOOL(cf_get_item_by_name("showfps"));
  conf.sound = CF_BOOL(cf_get_item_by_name("sound"));
  conf.a_btn = CF_VAL(cf_get_item_by_name("a_btn"));
  conf.b_btn = CF_VAL(cf_get_item_by_name("b_btn"));
  conf.x_btn = CF_VAL(cf_get_item_by_name("x_btn"));
  conf.y_btn = CF_VAL(cf_get_item_by_name("y_btn"));
  conf.l_btn = CF_VAL(cf_get_item_by_name("l_btn"));
  conf.r_btn = CF_VAL(cf_get_item_by_name("r_btn"));
  conf.system = SYS_ARCADE;
  conf.country = CTY_EUROPE;
}

static void read_array(int *tab, char *val, int size)
{
  int i = 0;
  char *v;

  v = strtok(val, ",");

  while(v != NULL && i < size) {
    tab[i] = atoi(v);
    v = strtok(NULL, ",");
    i++;
  }
}

static char **read_str_array(char *val, int *size)
{
  char *v;
  int nb_elem = 1;
  int i = 0;
  char **tab;
  while(val[i] != 0) {
    if(val[i] == ',') {
      nb_elem++;
    }
    i++;
  }
  printf("%s :NB elem %d\n", val, nb_elem);
  tab = malloc(nb_elem * sizeof(char *));
  if(!tab) {
    return NULL;
  }

  v = strtok(val, ",");
  printf("V1=%s\n", v);
  for(i = 0; i < nb_elem; i++) {
    tab[i] = strdup(v);
    v = strtok(NULL, ",");
    printf("V%d=%s\n", i, v);
  }
  *size = nb_elem;
  return tab;
}

static CONF_ITEM *create_conf_item(const char *name, const char *help, char short_opt, int (*action)(struct CONF_ITEM *self))
{
  int a;
  static int val = 0x100;
  CONF_ITEM *t = (CONF_ITEM *) calloc(1, sizeof(CONF_ITEM));

  a = tolower((int) name[0]);

  t->name = strdup(name);
  t->help = strdup(help);
  t->modified = 0;
  if(short_opt == 0) {
    val++;
    t->short_opt = val;
  }
  else {
    t->short_opt = short_opt;
  }

  if(action) {
    t->action = action;
  }


  if(cf_hash[a].size <= cf_hash[a].nb_item) {
    cf_hash[a].size += 10;
    cf_hash[a].conf = (CONF_ITEM **) realloc(cf_hash[a].conf, cf_hash[a].size * sizeof(CONF_ITEM *));
  }

  cf_hash[a].conf[cf_hash[a].nb_item] = t;
  cf_hash[a].nb_item++;
  return t;
}

void cf_create_bool_item(const char *name, const char *help, char short_opt, int def)
{
  CONF_ITEM *t = create_conf_item(name, help, short_opt, NULL);
  t->type = CFT_BOOLEAN;
  t->data.dt_bool.boolean = def;
  t->data.dt_bool.default_bool = def;
}

void cf_create_action_item(const char *name, const char *help, char short_opt, int (*action)(struct CONF_ITEM *self))
{
  CONF_ITEM *t = create_conf_item(name, help, short_opt, action);
  t->type = CFT_ACTION;
}

void cf_create_action_arg_item(const char *name, const char *help, const char *hlp_arg, char short_opt, int (*action)(struct CONF_ITEM *self))
{
  CONF_ITEM *t = create_conf_item(name, help, short_opt, action);
  t->type = CFT_ACTION_ARG;
  t->help_arg = (char *) hlp_arg;
}

void cf_create_string_item(const char *name, const char *help, const char *hlp_arg, char short_opt, const char *def)
{
  CONF_ITEM *t = create_conf_item(name, help, short_opt, NULL);
  t->type = CFT_STRING;
  strcpy(t->data.dt_str.str, def);
  t->data.dt_str.default_str = strdup(def);
  t->help_arg = (char *) hlp_arg;
}

void cf_create_int_item(const char *name, const char *help, const char *hlp_arg, char short_opt, int def)
{
  CONF_ITEM *t = create_conf_item(name, help, short_opt, NULL);
  t->type = CFT_INT;
  t->data.dt_int.val = def;
  t->data.dt_int.default_val = def;
  t->help_arg = (char *) hlp_arg;
}

void cf_create_array_item(const char *name, const char *help, const char *hlp_arg, char short_opt, int size, int *def)
{
  CONF_ITEM *t = create_conf_item(name, help, short_opt, NULL);
  t->type = CFT_ARRAY;
  t->data.dt_array.size = size;
  t->data.dt_array.array = (int *) calloc(1, size * sizeof(int));
  memcpy(t->data.dt_array.array, def, size * sizeof(int));
  t->data.dt_array.default_array = def;
  t->help_arg = (char *) hlp_arg;
}

void cf_create_str_array_item(const char *name, const char *help, const char *hlp_arg, char short_opt, char *def)
{
  CONF_ITEM *t = create_conf_item(name, help, short_opt, NULL);
  t->type = CFT_STR_ARRAY;
  t->data.dt_str_array.size = 0; /* Calculated on the fly */
  if(def != NULL) {
    t->data.dt_str_array.array = read_str_array(def, &t->data.dt_str_array.size);
  }
  else {
    t->data.dt_str_array.array = NULL;
  }
  t->data.dt_str_array.default_array = strdup(def);
  t->help_arg = (char *) hlp_arg;
}

CONF_ITEM *cf_get_item_by_name(const char *name)
{
  int i;
  int a = tolower((int) name[0]);

  if(a >= 128) {
    return NULL;
  }

  for(i = 0; i < cf_hash[a].nb_item; i++) {
    if(strcasecmp(cf_hash[a].conf[i]->name, name) == 0) {
      return cf_hash[a].conf[i];
    }
  }
  return NULL;
}

CONF_ITEM *cf_get_item_by_val(int val)
{
  int i, j;

  for(i = 0; i < 128; i++) {
    for(j = 0; j < cf_hash[i].nb_item; j++) {
      if(cf_hash[i].conf[j]->short_opt == val) {
        return cf_hash[i].conf[j];
      }
    }
  }
  return NULL;
}

void cf_reset_all_changed_flag()
{
  int i, j;

  for(i = 0; i < 128; i++)
    for(j = 0; j < cf_hash[i].nb_item; j++) {
      cf_hash[i].conf[j]->modified = 0;
    }
}

void cf_item_has_been_changed(CONF_ITEM *item)
{
  if(item) {
    item->modified = 1;
  }
}

void cf_init(void)
{
  //char *lr_btn_string[] = {"None", "A", "B", "C", "D", "A+B", "A+C", "A+D", "B+C", "B+D", "C+D", "A+B+C", "A+B+D", "A+C+D", "B+C+D", "A+B+C+D"};
  cf_create_bool_item("showfps", "Show FPS", 0, GN_FALSE);
  cf_create_bool_item("sound", "Enable Sound", 0, GN_TRUE);
  cf_create_int_item("a_btn", "Set Custom Button", "A", 0, 1);
  cf_create_int_item("b_btn", "Set Custom Button", "B", 0, 2);
  cf_create_int_item("x_btn", "Set Custom Button", "X", 0, 3);
  cf_create_int_item("y_btn", "Set Custom Button", "Y", 0, 4);
  cf_create_int_item("l_btn", "Set Custom Button", "L", 0, 0);
  cf_create_int_item("r_btn", "Set Custom Button", "R", 0, 0);
  cf_create_string_item("rompath", "Tell gngeo where your roms are", "PATH", 'i', "/mnt/roms/NEOGEO");
  cf_create_bool_item("dump", "Create a gno dump in the current dir and exit", 0, GN_FALSE);
  cf_get_item_by_name("rompath")->flags |= CF_SYSTEMOPT;
}

int discard_line(char *buf)
{
  if(buf[0] == '#') {
    return GN_TRUE;
  }
  if(buf[0] == '\n') {
    return GN_TRUE;
  }
  if(buf[0] == 0) {
    return GN_TRUE;
  }

  return GN_FALSE;
}

/* like standard fgets, but work with unix/dos line ending */
char *my_fgets(char *s, int size, FILE *stream)
{
  int i = 0;
  int ch;
  while(i < size && !feof(stream)) {
    ch = fgetc(stream); //printf("ch=%d\n",ch);
    if(ch == 0x0D) {
      continue;
    }
    if(ch == 0x0A) {
      s[i] = 0;
      return s;
    }
    s[i] = ch;
    i++;
  }
  return s;
}



int cf_save_option(char *filename, char *optname, int flags)
{
  char *conf_file = filename;
  char *conf_file_dst;
  FILE *f;
  FILE *f_dst;
  int i = 0, j, a;
  char buf[512] = {0};
  char name[32] = {0};
  char val[255] = {0};
  CONF_ITEM *cf;
  CONF_ITEM *tosave; //cf_get_item_by_name(optname);

  if(!conf_file) {
    int len = strlen("gngeorc") + strlen(getenv("HOME")) + strlen("/.gngeo/") + 1;
    conf_file = (char *) alloca(len * sizeof(char));
    sprintf(conf_file, "%s/.gngeo/gngeorc", getenv("HOME"));
  }
  conf_file_dst = alloca(strlen(conf_file) + 4);
  sprintf(conf_file_dst, "%s.t", conf_file);

  if((f_dst = fopen(conf_file_dst, "w")) == 0) {
    return GN_FALSE;
  }
  if(optname != NULL) {
    tosave = cf_get_item_by_name(optname);
    if(tosave) {
      cf_item_has_been_changed(tosave);
    }
  }
  else {
    tosave = NULL;
  }

  if((f = fopen(conf_file, "rb"))) {
    while(!feof(f)) {
      i = 0;
      my_fgets(buf, 510, f);
      if(discard_line(buf)) {
        fprintf(f_dst, "%s\n", buf);
        continue;
      }

      sscanf(buf, "%s ", name);
      strncpy(val, buf + strlen(name) + 1, 254);

      printf("item name: %s\n", name);
      cf = cf_get_item_by_name(name);
      if(cf && (cf == tosave || tosave == NULL)) {
        if(cf->modified) {
          cf->modified = 0;
          switch(cf->type) {
          case CFT_INT:
            fprintf(f_dst, "%s %d\n", cf->name, CF_VAL(cf));
            break;
          case CFT_BOOLEAN:
            if(CF_BOOL(cf)) {
              fprintf(f_dst, "%s true\n", cf->name);
            }
            else {
              fprintf(f_dst, "%s false\n", cf->name);
            }
            break;
          case CFT_STRING:
            fprintf(f_dst, "%s %s\n", cf->name, CF_STR(cf));
            break;
          case CFT_ARRAY:
            fprintf(f_dst, "%s ", cf->name);
            for(a = 0; a < CF_ARRAY_SIZE(cf) - 1; a++) {
              fprintf(f_dst, "%d,", CF_ARRAY(cf)[a]);
            }
            fprintf(f_dst, "%d\n", CF_ARRAY(cf)[a]);
            break;
          case CFT_ACTION:
          case CFT_ACTION_ARG:
            break;
          case CFT_STR_ARRAY:
            printf("TODO: Save CFT_STR_ARRAY\n");
            break;
          }
        }
        else {
          fprintf(f_dst, "%s\n", buf);
        }
      }
    }
    fclose(f);

  }
  for(i = 0; i < 128; i++) {
    for(j = 0; j < cf_hash[i].nb_item; j++) {
      cf = cf_hash[i].conf[j];
      if(cf->modified != 0  && (cf == tosave || tosave == NULL)) {
        cf->modified = 0;
        switch(cf->type) {
        case CFT_INT:
          fprintf(f_dst, "%s %d\n", cf->name, CF_VAL(cf));
          break;
        case CFT_BOOLEAN:
          if(CF_BOOL(cf)) {
            fprintf(f_dst, "%s true\n", cf->name);
          }
          else {
            fprintf(f_dst, "%s false\n", cf->name);
          }
          break;
        case CFT_STRING:
          fprintf(f_dst, "%s %s\n", cf->name, CF_STR(cf));
          break;
        case CFT_ARRAY:
          fprintf(f_dst, "%s ", cf->name);
          for(a = 0; a < CF_ARRAY_SIZE(cf) - 1; a++) {
            fprintf(f_dst, "%d,", CF_ARRAY(cf)[a]);
          }
          fprintf(f_dst, "%d\n", CF_ARRAY(cf)[a]);
          break;
        case CFT_ACTION:
        case CFT_ACTION_ARG:
          break;
        case CFT_STR_ARRAY:
          printf("TODO: Save CFT_STR_ARRAY\n");
          break;
        }
      }
    }
  }
  fclose(f_dst);
  remove(conf_file);
  rename(conf_file_dst, conf_file);
  return GN_TRUE;
}

int cf_save_file(char *filename, int flags)
{
  return cf_save_option(filename, NULL, flags);
}

void cf_reset_to_default(void)
{
  int i, j;
  CONF_ITEM *cf;
  for(i = 0; i < 128; i++) {
    for(j = 0; j < cf_hash[i].nb_item; j++) {
      cf = cf_hash[i].conf[j];
      if(!cf->modified && !(cf->flags & CF_SETBYCMD) && !(cf->flags & CF_SYSTEMOPT)) {
        switch(cf->type) {
        case CFT_INT:
          CF_VAL(cf) = cf->data.dt_int.default_val;
          break;
        case CFT_BOOLEAN:
          CF_BOOL(cf) = cf->data.dt_bool.default_bool;
          break;
        case CFT_STRING:
          strncpy(CF_STR(cf), cf->data.dt_str.default_str, 254);
          break;
        case CFT_ARRAY:
          memcpy(cf->data.dt_array.array, cf->data.dt_array.default_array,
                 CF_ARRAY_SIZE(cf) * sizeof(int));
          //read_array(CF_ARRAY(cf), val, CF_ARRAY_SIZE(cf));
          break;
        default:
          break;
        }
      }
    }
  }
}

int cf_open_file(char *filename)
{
  /* if filename==NULL, we use the default one: $HOME/.gngeo/gngeorc */
  char *conf_file = filename;
  FILE *f;
  int i = 0;
  char buf[512];
  char name[32];
  char val[255];
  CONF_ITEM *cf;

  if(!conf_file) {
    int len = strlen("gngeorc") + strlen(getenv("HOME")) + strlen("/.gngeo/") + 1;
    conf_file = (char *) alloca(len * sizeof(char));
    sprintf(conf_file, "%s/.gngeo/gngeorc", getenv("HOME"));
  }
  if((f = fopen(conf_file, "rb")) == 0) {
    //printf("Unable to open %s\n",conf_file);
    return GN_FALSE;
  }

  while(!feof(f)) {
    i = 0;
    my_fgets(buf, 510, f);
    if(discard_line(buf)) {
      continue;
    }

    sscanf(buf, "%s %s", name, val);
    cf = cf_get_item_by_name(name);
    if(cf && !(cf->flags & CF_SETBYCMD) && (!cf->modified)) {
      //printf("Option %s\n", cf->name);
      switch(cf->type) {
      case CFT_INT:
        CF_VAL(cf) = atoi(val);
        break;
      case CFT_BOOLEAN:
        CF_BOOL(cf) = (strcasecmp(val, "true") == 0 ? GN_TRUE : GN_FALSE);
        break;
      case CFT_STRING:
        strncpy(CF_STR(cf), val, 254);
        break;
      case CFT_ARRAY:
        read_array(CF_ARRAY(cf), val, CF_ARRAY_SIZE(cf));
        break;
      case CFT_ACTION:
      case CFT_ACTION_ARG:
        /* action are not available in the conf file */
        break;
      case CFT_STR_ARRAY:
        CF_STR_ARRAY(cf) = read_str_array(val, &CF_STR_ARRAY_SIZE(cf));
        break;
      }
    }
    else {
      /*printf("Unknow option %s\n",name);*/
      /* unknow option...*/
    }
  }

  cf_cache_conf();
  return GN_TRUE;
}


static struct option *longopt;
//static struct option *fake_longopt;

static void add_long_opt_item(char *name, int has_arg, int *flag, int val)
{
  static int opt_size = 0;
  static int opt = 0;

  if(opt >= opt_size) {
    opt_size += 10;
    longopt = realloc(longopt, (opt_size + 1) * sizeof(struct option));
    //fake_longopt=realloc(fake_longopt,(opt_size+1)*sizeof(struct option));
  }

  longopt[opt].name = name;
  longopt[opt].has_arg = has_arg;
  longopt[opt].flag = flag;
  longopt[opt].val = val;

  /*
  	fake_longopt[opt].name=name;
  	fake_longopt[opt].has_arg=has_arg;
  	fake_longopt[opt].flag=NULL;
  	fake_longopt[opt].val=0;
   */
  opt++;
}

int option_index = 0;
static char shortopt[255];

void cf_init_cmd_line(void)
{
  int i, j;
  CONF_ITEM *cf;
  char *sbuf;
  int buflen;

  memset(shortopt, 0, 255);

  for(i = 0; i < 128; i++) {
    for(j = 0; j < cf_hash[i].nb_item; j++) {
      cf = cf_hash[i].conf[j];

      if(cf->short_opt <= 128) {
        char b[2];
        sprintf(b, "%c", cf->short_opt);
        strcat(shortopt, b);
      }

      switch(cf->type) {
      case CFT_ARRAY:
      case CFT_STR_ARRAY:
      case CFT_STRING:
      case CFT_INT:
      case CFT_ACTION_ARG:
        if(cf->short_opt <= 128) {
          strcat(shortopt, ":");
        }
        add_long_opt_item(cf->name, 1, NULL, cf->short_opt);
        break;
      case CFT_BOOLEAN:
        //add_long_opt_item(cf->name, 0, &CF_BOOL(cf), 1);
        add_long_opt_item(cf->name, 0, NULL, cf->short_opt);
        /* create the --no-option */
        buflen = strlen("no-") + strlen(cf->name);
        sbuf = malloc(buflen + 2);
        snprintf(sbuf, buflen + 1, "no-%s", cf->name);
        //add_long_opt_item(sbuf, 0, &CF_BOOL(cf), 0);
        add_long_opt_item(sbuf, 0, NULL, cf->short_opt + 0x1000);
        break;
      case CFT_ACTION:
        add_long_opt_item(cf->name, 0, NULL, cf->short_opt);
        break;
      }
    }/* for j*/
  }/* for i*/

  /* end the longopt array*/
  add_long_opt_item(0, 0, 0, 0);

}

char *cf_parse_cmd_line(int argc, char *argv[])
{
  int c;
  CONF_ITEM *cf;


  option_index = optind = 0;
#ifdef WII
  return NULL;
#endif
  while((c = getopt_long(argc, argv, shortopt, longopt, &option_index)) != EOF) {
    //if (c != 0) {
    printf("c=%d\n", c);
    cf = cf_get_item_by_val(c & 0xFFF);
    if(cf) {
      cf->flags |= CF_SETBYCMD;
      printf("flags %s set on cmd line\n", cf->name);
      switch(cf->type) {

      case CFT_INT:
        CF_VAL(cf) = atoi(optarg);
        break;
      case CFT_BOOLEAN:
        if(c & 0x1000) {
          CF_BOOL(cf) = 0;
        }
        else {
          CF_BOOL(cf) = 1;
        }
        break;
      case CFT_STRING:
        strcpy(CF_STR(cf), optarg);
        //printf("conf %s %s\n",CF_STR(cf),optarg);
        break;
      case CFT_ARRAY:
        read_array(CF_ARRAY(cf), optarg, CF_ARRAY_SIZE(cf));
        break;
      case CFT_ACTION_ARG:
        strcpy(CF_STR(cf), optarg);
        if(cf->action) {
          exit(cf->action(cf));
        }
        break;
      case CFT_ACTION:
        if(cf->action) {
          exit(cf->action(cf));
        }
        break;
      case CFT_STR_ARRAY:
        /* TODO */
        break;
      }
      //}
    }
  }
  cf_cache_conf();
  if(optind >= argc) {
    return NULL;
  }

  return strdup(argv[optind]);
}

