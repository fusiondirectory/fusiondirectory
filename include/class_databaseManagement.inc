<?php

/*
  This code is part of FusionDirectory (http://www.fusiondirectory.org/)
  Copyright (C) 2012  FusionDirectory

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
*/

/*!
 * \file class_databaseManagement.inc
 * Source code for class databasManagement
 */

require_once("MDB2.php");

/*!
 * \brief This class contains all database functions needed
 */
class databaseManagement {

  /*!
   * \brief This function connect to the database server and return the handle
   *
   * \param array $database Array with connection informations
   */
  static function connectDatabase($database)
  {
    $link =& MDB2::connect(array( 'phptype'=>'mysql',
                                  'hostspec'=>$database['SERVER'],
                                  'username'=>$database['LOGIN'],
                                  'password'=>$database['PASSWORD'],
                                  'database'=>$database['DB'],
                          ));
    return $link;
  }

  /*!
   * \brief This function execute a bunch of queries inside
   * a transaction and roll back if anything goes wrong
   *
   * \param $link Connection to MySQL Server
   *
   * \param String $queries Queries to execute
   */
  static function executeQueriesInTransaction($link,$queries)
  {
    /* Start transaction, to be able to rollback */
    @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,"<b>---Updating/Inserting entries---</b>","");

    $link->beginTransaction();
    @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,"<b>begin;</b>","<i>Starting transaction!</i>");

    foreach ($queries as $query) {
      @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,"<b>".$query."</b>", "");
      $res = $link->exec($query);
      if (PEAR::isError($res)) {
        $err = $res->getMessage();
        @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,"\n".$err, "<b>FAILED</b>");
        msg_dialog::display(_("Error"),
            msgPool::dbError($err,__CLASS__)."&nbsp;".
            "\n<p>"._("Please activate debugging for details!")."</p>",
            ERROR_DIALOG);

        $link->rollback();
        @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,"<b>rollback;</b>", "<b>ERROR</b> Rollback transaction!");
        $link->disconnect();
        return FALSE;
      }
    }

    /* Let changes get active, everything was fine */
    $link->commit();
    @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,"<b>commit;</b>", "");
    @DEBUG (DEBUG_DB, __LINE__, __FUNCTION__, __FILE__,"<b>---Transaction sucessful!---</b>", "");
    return TRUE;
  }
}

?>
