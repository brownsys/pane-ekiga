
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         gmcontacts-remote.cpp - description 
 *                         ------------------------------------
 *   begin                : Mon May 23 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Declaration of the remote addressbook access 
 *   			    functions. Use the API in gmcontacts.h instead.
 *
 */


/*
 * Remote users can be either LDAP or ILS or ZeroConf users
 */

#include "../../config.h"

#include <string.h>
#include <gmconf.h>

#include "gmcontacts.h"
#ifndef _GM_CONTACTS_H_INSIDE__
#define _GM_CONTACTS_H_INSIDE__
#include "gmcontacts-remote.h"
#include "gmcontacts-ldap.h"
#ifdef HAS_AVAHI
#include "gmcontacts-avahi.h"
#endif
#undef _GM_CONTACTS_H_INSIDE__
#endif


GSList *
gnomemeeting_get_remote_addressbooks ()
{
  GSList *l = NULL;

  l = gnomemeeting_get_ldap_addressbooks ();

#ifdef HAS_AVAHI
  GSList *h = NULL;

  h = gnomemeeting_get_zero_addressbooks ();
  l = g_slist_concat (l, h); /* No copy is done */
#endif
  
  return l;
}


GSList *
gnomemeeting_remote_addressbook_get_contacts (GmAddressbook *addressbook,
					      int &nbr,
					      gboolean partial_match,
					      const gchar *fullname,
					      const gchar *url,
					      const gchar *categorie,
					      const gchar *location,
					      const gchar *speeddial)
{
  if (addressbook && gnomemeeting_addressbook_is_ldap (addressbook)) 
    return gnomemeeting_ldap_addressbook_get_contacts (addressbook,
						       nbr,
						       partial_match,
						       fullname,
						       url,
						       categorie,
						       location,
						       speeddial);
#ifdef HAS_AVAHI /* If it is not an ldap addressbook, then it is a ZC one */
  else
    return gnomemeeting_zero_addressbook_get_contacts (addressbook,
						       nbr,
						       partial_match,
						       fullname,
						       url,
						       categorie,
						       location,
						       speeddial);
#else
  else
    return NULL;
#endif
}


gboolean 
gnomemeeting_remote_addressbook_add (GmAddressbook *addressbook)
{
  GSList *list = NULL;
  gchar *entry = NULL;
  
  list = 
    gm_conf_get_string_list ("/apps/" PACKAGE_NAME "/contacts/remote_addressbooks_list");

  entry = g_strdup_printf ("%s|%s|%s|%s", 
			   addressbook->aid, 
			   addressbook->name, 
			   addressbook->url,
			   addressbook->call_attribute);

  list = g_slist_append (list, (gpointer) entry);
  gm_conf_set_string_list ("/apps/" PACKAGE_NAME "/contacts/remote_addressbooks_list", 
			   list);

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  return TRUE;
}


gboolean 
gnomemeeting_remote_addressbook_delete (GmAddressbook *addressbook)
{
  GSList *list = NULL;
  GSList *l = NULL;
  
  gchar *entry = NULL;
  
  gboolean found = FALSE;
  
  list = 
    gm_conf_get_string_list ("/apps/" PACKAGE_NAME "/contacts/remote_addressbooks_list");

  entry = 
    g_strdup_printf ("%s|%s|%s|%s", 
		     addressbook->aid, 
		     addressbook->name, 
		     addressbook->url,
		     addressbook->call_attribute);

  l = list;
  while (l && !found) {

    if (l->data && !strcmp ((const char *) l->data, entry)) {

      found = TRUE;
      break;
    }
    
    l = g_slist_next (l);
  }
  
  if (found) {

    list = g_slist_remove_link (list, l);

    g_free (l->data);
    g_slist_free_1 (l);

    gm_conf_set_string_list ("/apps/" PACKAGE_NAME "/contacts/remote_addressbooks_list", 
			     list);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  g_free (entry);

  return found;
}


gboolean 
gnomemeeting_remote_addressbook_modify (GmAddressbook *addressbook)
{
  GSList *list = NULL;
  GSList *l = NULL;
  
  gchar *entry = NULL;
  gchar **couple = NULL;
  
  gboolean found = FALSE;
  
  list = 
    gm_conf_get_string_list ("/apps/" PACKAGE_NAME "/contacts/remote_addressbooks_list");

  entry = 
    g_strdup_printf ("%s|%s|%s|%s", 
		     addressbook->aid,
		     addressbook->name, 
		     addressbook->url,
		     addressbook->call_attribute);

  l = list;
  while (l && !found) {

    if (l->data) {
      
      couple = g_strsplit ((const char *) l->data, "|", 0);
      if (couple && couple [0] && !strcmp (couple [0], addressbook->aid)) {

	found = TRUE;
	break;
      }
    }
    
    l = g_slist_next (l);
  }
  
  if (found) {

    list = g_slist_insert_before (list, l, (gpointer) entry);
    list = g_slist_remove_link (list, l);

    g_free (l->data);
    g_slist_free_1 (l);


    gm_conf_set_string_list ("/apps/" PACKAGE_NAME "/contacts/remote_addressbooks_list", 
			     list);
  }

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  return found;
}


gboolean
gnomemeeting_remote_addressbook_is_editable (GmAddressbook *addressbook)
{
  if (addressbook && gnomemeeting_addressbook_is_ldap (addressbook))
    return TRUE;
  else
    return FALSE;
}


void
gnomemeeting_remote_addressbook_init ()
{
#ifdef HAS_AVAHI
  gnomemeeting_zero_addressbook_init ();
#endif
}


gboolean 
gnomemeeting_remote_addressbook_has_fullname (GmAddressbook *)
{
  return TRUE;
}


gboolean 
gnomemeeting_remote_addressbook_has_url (GmAddressbook *)
{
  return TRUE;
}


gboolean 
gnomemeeting_remote_addressbook_has_speeddial (GmAddressbook *)
{
  return FALSE;
}


gboolean 
gnomemeeting_remote_addressbook_has_categories (GmAddressbook *)
{
  return FALSE;
}


gboolean 
gnomemeeting_remote_addressbook_has_location (GmAddressbook *addressbook)
{
  if (addressbook && gnomemeeting_addressbook_is_ldap (addressbook))
    return TRUE;
  
  return FALSE;
}


gboolean 
gnomemeeting_remote_addressbook_has_comment (GmAddressbook *addressbook)
{
  if (addressbook && gnomemeeting_addressbook_is_ldap (addressbook))
    return TRUE;

  return FALSE;
}


gboolean 
gnomemeeting_remote_addressbook_has_software (GmAddressbook *addressbook)
{
  return TRUE;
}


gboolean 
gnomemeeting_remote_addressbook_has_email (GmAddressbook *addressbook)
{
  if (addressbook && gnomemeeting_addressbook_is_ldap (addressbook))
    return TRUE;

  return FALSE;
}


gboolean 
gnomemeeting_remote_addressbook_has_state (GmAddressbook *)
{
  return TRUE;
}

