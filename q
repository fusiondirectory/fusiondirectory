[1mdiff --git a/Changelog b/Changelog[m
[1mindex bf29f455..53c3bd2d 100644[m
[1m--- a/Changelog[m
[1m+++ b/Changelog[m
[36m@@ -1,206 +1,6 @@[m
 FusionDirectory changelog[m
 =========================[m
 [m
[31m-* FusionDirectory 1.0.8.5[m
[31m-[m
[31m-[Fix] Bugs #3249: When a duplicate is found, its dn should be given[m
[31m-[Feature] Wishlist #3264: Hidden password for fusiondirectory-setup --check-ldap[m
[31m-[Fix] Bugs #3315: Warning message during adduser process[m
[31m-[Feature] Wishlist #3397: Ppolicy plugin[m
[31m-[Fix] Bugs #3437: Handle password policy checking inside FD[m
[31m-[Feature] Wishlist #3477: use fai plugin without creating a repository debian bug.[m
[31m-[Feature] Wishlist #3482: Plugin FAI - Centos support[m
[31m-[Fix] Bugs #3489: Documentation pour installer les dépots RPM[m
[31m-[Fix] Bugs #3492: Country error[m
[31m-[Fix] Bugs #3507: Error in plugin-dns documentation[m
[31m-[Fix] Bugs #3521: Can’t give group creation right through ACL system[m
[31m-[Fix] Bugs #3524: Can't create/edit users with gosaMailServer Attributes via acl system[m
[31m-[Fix] Bugs #3525: the sudo plugin doesnt allow to save the sudoOrder[m
[31m-[Fix] Bugs #3527: Add the value ALL in systems and users and groups[m
[31m-[Fix] Bugs #3528: misleading dialog box when trying to recover password for a non-existant user[m
[31m-[Fix] Bugs #3533: Date d'expiration Unix and Samba[m
[31m-[Feature] Wishlist #3534: gosaMailDeliveryMode in Group Mail Tab[m
[31m-[Fix] Bugs #3536: Plugins Quota et msg :The field 'Device' contains invalid characters![m
[31m-[Fix] Bugs #3537: Error when I click on reference in EJBCA section[m
[31m-[Fix] Bugs #3538: getbin.php should quote the file name[m
[31m-[Fix] Bugs #3539: User tab roles should not appear in «My account» menu[m
[31m-[Fix] Bugs #3540: Password Recovery error for a non-existant user[m
[31m-[Fix] Bugs #3544: when adding ns record in the dns zone editor global name records it doesnt add the . at the end when saving[m
[31m-[Fix] Bugs #3547: Password Recovery (Invalid Token)[m
[31m-[Fix] Bugs #3551: Invalid uri in the password recovery mail[m
[31m-[Feature] Bugs #3554: New functionnality: list all members for a primary group[m
[31m-[Fix] Bugs #3555: editing a template inside fai provoke a crash[m
[31m-[Fix] Bugs #3557: Plugin FAI - partition LVM[m
[31m-[Fix] Bugs #3561: Remove the message "Take over DNS configuration from ..."[m
[31m-[Feature] simple-plugin - Bugs #3562: In most password fields, autocomplete should be deactivated[m
[31m-[Fix] Bugs #3564: References are only checked in the base[m
[31m-[Fix] Bugs #3569: after the first load of the kernel to be use in the fai tab, they should be cached for the rest of the session[m
[31m-[Fix] Bugs #3571: Informations for DHCP and DNS are lost after a copy paste[m
[31m-[Fix] Bugs #3579: We should update copyright notices to 2015[m
[31m-[Fix] Bugs #3580: We should update copyright notices to 2015[m
[31m-[Fix] Bugs #3582: translation not complete[m
[31m-[Fix] Wishlist #3586: Modifier for templates to convert german umlauts to 7-bit ASCII[m
[31m-[Fix] Bugs #3590: Error message if we have an assignement with no members[m
[31m-[Fix] Bugs #3592: the ppolicy exemple files should be changed to use a more generic dn[m
[31m-[Feature] Bugs #3595: The menu should be reorganized[m
[31m-[Feature] Bugs #3596: Dashboard plugin should go into core[m
[31m-[Feature] Bugs #3600: Dashboard should be completed[m
[31m-[Feature] Bugs #3601: The menu should be reorganized[m
[31m-[Fix] Bugs #3607: in queue management we should remove the action create and the first 3 icons on the left[m
[31m-[Feature] Bugs #3608: dashboard should be an exhaustive statistic plugin[m
[31m-[Fix] Bugs #3609: system and newtork tab from the dashboard plugin goes to systems plugins to add the tab when needed[m
[31m-[Feature] Bugs #3610: ppolicy should add a tab to dashboard[m
[31m-[Feature] Bugs #3616: Dashboard plugin should go into core[m
[31m-[Feature] Bugs #3620: system and newtork tab from the dashboard plugin goes to systems plugins to add the tab when needed[m
[31m-[Fix] Bugs #3621: ejbca my account link is broken[m
[31m-[Feature] Bugs #3622: ppolicy should add a tab to dashboard[m
[31m-[Fix] Bugs #3623: Cannot create a role with the same name of an DSA object[m
[31m-[Fix] simple-plugin - Bugs #3624: When a duplicate is found, its dn should be given[m
[31m-[Fix] Bugs #3626: Export single entry give an error[m
[31m-[Fix] Bugs #3627: OPSI import should not have an empty filter[m
[31m-[Fix] Bugs #3628: DHCP plugin edit host error[m
[31m-[Feature] Bugs #3630: Reset password must take the mail in personal plugin[m
[31m-[Fix] Bugs #3631: User templates issues.[m
[31m-[Fix] Bugs #3633: the small_warning.png icon is not present but used by the system dashboard tab[m
[31m-[Fix] Bugs #3634: the configuration of fusiondirectory entry in dashboard first tab doesnt have an icon[m
[31m-[Fix] Bugs #3635: the small_warning.png icon is not present but used by the system dashboard tab[m
[31m-[Fix] Bugs #3637: Applications plugin should not set forceSize parameter[m
[31m-[Fix] Bugs #3638: Error in cyrus template[m
[31m-[Fix] Bugs #3640: Remove a user display an error if we have ppolicy[m
[31m-[Fix] Bugs #3641: Token invalid when we use ppolicy[m
[31m-[Fix] Bugs #3642: FTPStatus attribute must default set at true[m
[31m-[Fix] Bugs #3644: Problem in regex to set release in FAI package list[m
[31m-[Fix] Bugs #3645: Parent servers are empty[m
[31m-[Fix] Bugs #3648: debconf variables field should take utf8[m
[31m-[Fix] Bugs #3653: Uninitialized string offset: 0 in ldapmanager when importing an ldif[m
[31m-[Fix] Bugs #3654: fai plugin empty with one repository on a server trigger an Undefined variable: prefix error[m
[31m-[Fix] Bugs #3659: error when copying a system from a departement to the root[m
[31m-[Fix] Bugs #3660: error when having two repo in one serveur and one repo in another system[m
[31m-[Fix] Bugs #3663: No image for up/down in FAI profil[m
[31m-[Fix] Bugs #3666: the dashboard doesnt show the icon associated to the fai objects and are not clickable[m
[31m-[Feature] Bugs #3667: the reset password should use the fdPrivateMail from the personnal plugin as alternative address[m
[31m-[Fix] Bugs #3669: in service we should not have the get status button if argonaut client tab is not activated[m
[31m-[Fix] Bugs #3670: in service we could add a second button to trigger an action on the selected services[m
[31m-[Fix] Bugs #3671: the reset password should use the fdPrivateMail from the personal plugin as alternative address[m
[31m-[Fix] Bugs #3676: remove example.ldif that is incorrect[m
[31m-[m
[31m-* FusionDirectory 1.0.8.4[m
[31m-[m
[31m-[Fix] Bugs #3530: the roles in the personal/roles miss an main.inc ![m
[31m-[Fix] Bugs #3531: the roles icon is missing in my account[m
[31m-[m
[31m-* FusionDirectory 1.0.8.3[m
[31m-[m
[31m-[Feature] Wishlist #258: PKI infrastructure ?[m
[31m-[Fix] Bugs #2613: new icon for debug-help aka diagrams has it is the same as ldapmanager[m
[31m-[Fix] Bugs #3330: fai ldap support should be send to the fai project[m
[31m-[Fix] Bugs #3368: Missing png for goServer (systems)[m
[31m-[Fix] Bugs #3369: «&amp;» should be used in icon paths[m
[31m-[Fix] Bugs #3379: The quota tab must have a dependance on the unix tab[m
[31m-[Fix] Bugs #3387: When locking an account with no samba password it adds a samba attribute to it[m
[31m-[Fix] Bugs #3393: update-from-1.0.6-to-1.0.7 in plugin-systems[m
[31m-[Fix] Bugs #3396: remove old gosa spec from core-fd.schema[m
[31m-[Fix] Bugs #3398: the field description is not show in the user list[m
[31m-[Fix] Bugs #3399: sometime when refreshing the generic page on a user a get template placeholder in login[m
[31m-[Fix] Bugs #3400: copyright and license is missing for include/functions_debug.inc[m
[31m-[Fix] Bugs #3401: autofs/admin/autofs/main.inc have a gpl v3 licence and its wrong it should be gpl v2[m
[31m-[Fix] Bugs #3402: remove the Authors.rst from the autofs plugin[m
[31m-[Fix] Bugs #3403: remove the glp v3 licence from the autofs plugin[m
[31m-[Fix] Bugs #3404: remove the sieve dir from the mail plugin[m
[31m-[Fix] Bugs #3407: plugins are still using obsolete attribute cellspacing on table tags[m
[31m-[Fix] Bugs #3408: update-from-1.0.6-to-1.0.7 in plugin-mail[m
[31m-[Fix] Bugs #3415: application listing in the desktop plugin doesnt show the icons on the listing[m
[31m-[Feature] Bugs #3416: create a new web-application plugin[m
[31m-[Fix] Bugs #3417: the application tab crash in groups when i click on the menu tab[m
[31m-[Fix] Bugs #3421: memory exhaustion when supann plugin is installed and i try to use the import csv[m
[31m-[Fix] Bugs #3422: when i create a template in the user list i got a missing icon for the new user from template[m
[31m-[Fix] Bugs #3423: cleaning the desktop-management plugin[m
[31m-[Fix] Bugs #3426: cleaning the desktop-management plugin[m
[31m-[Fix] Bugs #3427: in some user case we have a need to be able to add roles for users and have those users get rights for applications.[m
[31m-[Fix] Bugs #3430: fusiondirectory-insert-schema man pages is wrong it misses an OPTIONS section[m
[31m-[Fix] Bugs #3431: correct all the date and version on the man pages[m
[31m-[Fix] Bugs #3433: when importing user and no selection of an field in the csv to be the uid it crash[m
[31m-[Fix] Bugs #3435: when importing user via csv and using a template i cannot put %mail% in the template because it reject it as a non valid address[m
[31m-[Fix] Bugs #3436: when using apply template after having sucessfully imported user, it wrongly rewrite the users entries[m
[31m-[Fix] Bugs #3442: dashboard network tab links to dns servers are wrong[m
[31m-[Fix] Bugs #3445: when saving a webapp with the applications plugin it doesnt show unless i click Search in subtrees[m
[31m-[Fix] Bugs #3446: when creating a desktop application it doesnt save it[m
[31m-[Fix] Bugs #3447: When debug trace level is on, we can’t log in anymore[m
[31m-[Feature] Bugs #3450: create a plugin to manage certificates from ejbca[m
[31m-[Fix] Bugs #3454: Error in the systems plugin documentation[m
[31m-[Fix] Bugs #3455: the supann plugin doesn't want to save the etablissement field[m
[31m-[Fix] Bugs #3457: when a create an etablissement with the supann strcuture plugin it create an object that is opened with the entity dialog[m
[31m-[Fix] Bugs #3458: making the saving of the samba attributes optional in 1.0.8.x[m
[31m-[Fix] Bugs #3459: the nis netgroup is always active[m
[31m-[Fix] Bugs #3460: Fusiondirectory access[m
[31m-[Fix] Bugs #3461: in the supann plugin the step double select and educational Element[m
[31m-[Fix] Bugs #3462: update the official files for the supann plugin[m
[31m-[Fix] Bugs #3468: Hook script with ldap array[m
[31m-[Fix] Bugs #3470: FusionDirectory in Amazon Linux AMI[m
[31m-[Feature] Bugs #3472: create a personal plugin to store all kind of personal data[m
[31m-[Fix] Bugs #3476: We should remove the ACL tabs from special tabs[m
[31m-[Fix] Bugs #3483: removing leftover of the desktop-management plugin[m
[31m-[Fix] Bugs #3484: The user icon does not show up[m
[31m-[Fix] Bugs #3485: The user icon does not show up[m
[31m-[Fix] Bugs #3490: GPG plugin have some errors[m
[31m-[Fix] Bugs #3491: User ldap error are wrongly reported[m
[31m-[Fix] Bugs #3493: Department list should be updated after a department creation[m
[31m-[Fix] Bugs #3495: foreignKey on acls is handle several times[m
[31m-[Fix] Bugs #3496: EJBCA tab for the systems[m
[31m-[Fix] Bugs #3497: EJBCA miss some icons[m
[31m-[Fix] Bugs #3500: Error in user management[m
[31m-[Fix] Bugs #3501: the uid id naming mess should be cleaned up[m
[31m-[Fix] Bugs #3502: Argonaut is sometimes called «infastructure service» and sometimes «service infrastructure»[m
[31m-[Fix] Bugs #3504: FD should stop using the word generic every where[m
[31m-[Fix] Bugs #3505: setup_feedback.tpl is a useless file[m
[31m-[Fix] Bugs #3506: we should have in core a tab in user to be able to assing him roles[m
[31m-[Fix] Bugs #3508: There is some french in password recovery[m
[31m-[Fix] Bugs #3510: Argonaut mirror settings fields have no descriptions[m
[31m-[Fix] Bugs #3511: The comment for the ssh public key is mandatory[m
[31m-[Fix] Bugs #3512: Application local seems wrong[m
[31m-[Fix] Bugs #3514: Various problems in strings[m
[31m-[Fix] Bugs #3516: Various problems in strings[m
[31m-[Fix] Bugs #3517: remove gouvernement mode from fusiondirectory[m
[31m-[Fix] Bugs #3518: remove the french inside the supann plugin[m
[31m-[Fix] Bugs #3519: Where we add users in sudo plugin their are invisible[m
[31m-[Fix] Bugs #3520: the default entry in the sudo plugin is not editable anymore[m
[31m-[Fix] Bugs #3522: the sudo schema from the sudo-ldap package i using old openldap syntax and must be corrected and included[m
[31m-[Fix] Bugs #3523: SASL exop and SASL relam can't be saved[m
[31m-[Feature] Wishlist #3419: FD should allow plugins to act as menu entries providers[m
[31m-[Feature] Wishlist #3429: fusiondirectory-insert-schema should have an option to keep ldifs files[m
[31m-[Feature] Wishlist #3441: Inactive tabs could have a different style[m
[31m-[Feature] Wishlist #3478: Message d'information concernant les propriétes que le mot de passe doit répondre[m
[31m-[Feature] Wishlist #3480: Question : uid : comment ca se passe ?[m
[31m-[Feature] Wishlist #3509: Should french translation use «courriel»?[m
[31m-[m
[31m-* FusionDirectory 1.0.8.2[m
[31m-[m
[31m-[Fix] Bugs #1034: remove all dsc form the sources[m
[31m-[Feature] Bugs #2619: fusiondirectory-insert-schema should have a -c option to continue on error[m
[31m-[Fix] Bugs #3289: error: PHP error: Undefined index: o (/usr/share/fusiondirectory/plugins/personal/supann/class_supannAccount.inc, line 481)[m
[31m-[Fix] Bugs #3297: Use relative path for geticon[m
[31m-[Fix] Bugs #3303: debugHelp have problems with tabclasses contaning dashes[m
[31m-[Feature] Bugs #3304: fusiondirectory-shell should have login and password mandatory[m
[31m-[Fix] Bugs #3316: possible xss in login screen[m
[31m-[Fix] Bugs #3321: Cannot import csv using a template[m
[31m-[Fix] Bugs #3327: primary server in the dns service should not be numeric[m
[31m-[Feature] Bugs #3335: adding lighttpd support in the doc[m
[31m-[Fix] Bugs #3338: informations are misses in reference tab on user interface[m
[31m-[Fix] Bugs #3347: update the doxygen file[m
[31m-[Fix] Bugs #3348: add the licence to class_IconTheme.inc[m
[31m-[Fix] Bugs #3350: correct the fsf address in licenses[m
[31m-[Fix] Bugs #3351: lots of plugins files have no licenses at all[m
[31m-[Fix] Bugs #3352: lots of plugins files have no licenses at all[m
[31m-[Fix] Bugs #3356: trust selection mode is broken in group and ogroups[m
[31m-[Fix] Bugs #3359: add user with templates didn't keep group affectation[m
[31m-[Fix] Bugs #3361: unable to create a template with posixGroup[m
[31m-[Fix] Bugs #3362: Bad aspect for tabs when a dialog is open[m
[31m-[Fix] Bugs #3363: cannont add Etablissement[m
[31m-[Fix] Bugs #3364: we can’t cancel manager selection[m
[31m-[Fix] Bugs #3365: there is an error in userManagement template base regexp[m
[31m-[Fix] Bugs #3372: remove the trust section from the sudo plugin[m
[31m-[Fix] Bugs #3373: Default action displayed an error[m
[31m-[m
 * FusionDirectory 1.0.8.1[m
 [m
 [Fix] Bugs #2702: Connection problems to slapd when using TLS[m
[1mdiff --git a/Changelog.md b/Changelog.md[m
[1mindex aff241eb..57b5dccd 100644[m
[1m--- a/Changelog.md[m
[1m+++ b/Changelog.md[m
[36m@@ -1637,3 +1637,252 @@[m
 #### fusiondirectory-plugins[m
 - fd-plugins#3687 Address book plugin show infos only in admin[m
 - fd-plugins#3719 ACLs of GPG-Plugin[m
[32m+[m
[32m+[m[32m## %"FusionDirectory 1.0.8.5" - 2015-04-03[m
[32m+[m
[32m+[m[32m### Added[m
[32m+[m
[32m+[m[32m#### fusiondirectory[m
[32m+[m[32m- fd#3554 New functionnality: list all members for a primary group[m
[32m+[m[32m- fd#3586 Modifier for templates to convert german umlauts to 7-bit ASCII[m
[32m+[m[32m- fd#3596 Dashboard plugin should go into core[m
[32m+[m[32m- fd#3600 Dashboard should be completed[m
[32m+[m[32m- fd#3634 the configuration of fusiondirectory entry in dashboard first tab doesnt have an icon[m
[32m+[m[32m- fd#3667 the reset password should use the fdPrivateMail from the personnal plugin as alternative address[m
[32m+[m
[32m+[m[32m#### fusiondirectory-plugins[m
[32m+[m[32m- fd-plugins#3397 Ppolicy plugin[m
[32m+[m[32m- fd-plugins#3477 Plugin FAI[m
[32m+[m[32m- fd-plugins#3482 Plugin FAI - Centos support[m
[32m+[m[32m- fd-plugins#3525 the sudo plugin doesnt allow to save the sudoOrder[m
[32m+[m[32m- fd-plugins#3527 Add the value ALL in systems and users and groups[m
[32m+[m[32m- fd-plugins#3529 add supannRefId in supann Plugin[m
[32m+[m[32m- fd-plugins#3534 gosaMailDeliveryMode in Group Mail Tab[m
[32m+[m[32m- fd-plugins#3544 when adding ns record in the dns zone editor global name records it doesnt add the . at the end when saving[m
[32m+[m[32m- fd-plugins#3609 system and newtork tab from the dashboard plugin goes to systems plugins to add the tab when needed[m
[32m+[m[32m- fd-plugins#3610 ppolicy should add a tab to dashboard[m
[32m+[m[32m- fd-plugins#3671 the reset password should use the fdPrivateMail from the personal plugin as alternative address[m
[32m+[m
[32m+[m[32m### Changed[m
[32m+[m
[32m+[m[32m#### fusiondirectory[m
[32m+[m[32m- fd#3264 Hidden password for fusiondirectory-setup --check-ldap[m
[32m+[m[32m- fd#3538 getbin.php should quote the file name[m
[32m+[m[32m- fd#3539 User tab roles should not appear in «My account» menu[m
[32m+[m[32m- fd#3579 We should update copyright notices to 2015[m
[32m+[m[32m- fd#3595 The menu should be reorganized[m
[32m+[m[32m- fd#3608 dashboard should be an exhaustive statistic plugin[m
[32m+[m[32m- fd#3622 ppolicy should add a tab to dashboard[m
[32m+[m
[32m+[m[32m#### fusiondirectory-plugins[m
[32m+[m[32m- fd-plugins#3569 after the first load of the kernel to be use in the fai tab, they should be cached for the rest of the session[m
[32m+[m[32m- fd-plugins#3580 We should update copyright notices to 2015[m
[32m+[m[32m- fd-plugins#3601 The menu should be reorganized[m
[32m+[m[32m- fd-plugins#3645 Parent servers are empty[m
[32m+[m[32m- fd-plugins#3648 debconf variables field should take utf8[m
[32m+[m[32m- fd-plugins#3666 the dashboard doesnt show the icon associated to the fai objects and are not clickable[m
[32m+[m
[32m+[m[32m### Removed[m
[32m+[m
[32m+[m[32m#### fusiondirectory[m
[32m+[m[32m- fd#3620 system and newtork tab from the dashboard plugin goes to systems plugins to add the tab when needed[m
[32m+[m[32m- fd#3633 the small_warning.png icon is not present but used by the system dashboard tab[m
[32m+[m
[32m+[m[32m#### fusiondirectory-plugins[m
[32m+[m[32m- fd-plugins#3561 Remove the message "Take over DNS configuration from  ..."[m
[32m+[m[32m- fd-plugins#3607 in queue management we should remove the action create and the first 3 icons on the left[m
[32m+[m[32m- fd-plugins#3616 Dashboard plugin should go into core[m
[32m+[m[32m- fd-plugins#3635 the small_warning.png icon is not present but used by the system dashboard tab[m
[32m+[m[32m- fd-plugins#3654 fai plugin empty with one repository on a server trigger an Undefined variable: prefix error[m
[32m+[m[32m- fd-plugins#3676 remove example.ldif that is incorrect[m
[32m+[m
[32m+[m[32m### Fixed[m
[32m+[m
[32m+[m[32m#### fusiondirectory[m
[32m+[m[32m- fd#3249 When a duplicate is found, its dn should be given[m
[32m+[m[32m- fd#3315 Warning message during adduser process[m
[32m+[m[32m- fd#3524 Can't create/edit users with gosaMailServer Attributes via acl system[m
[32m+[m[32m- fd#3540 Password Recovery error for a non-existant user[m
[32m+[m[32m- fd#3551 Invalid uri in the password recovery mail[m
[32m+[m[32m- fd#3564 References are only checked in the base[m
[32m+[m[32m- fd#3582 translation not complete[m
[32m+[m[32m- fd#3590 Error message if we have an assignement with no members[m
[32m+[m[32m- fd#3623 Cannot create a role with the same name of an DSA object[m
[32m+[m[32m- fd#3659 error when copying a system from a departement to the root[m
[32m+[m
[32m+[m[32m#### fusiondirectory-plugins[m
[32m+[m[32m- fd-plugins#3507 Error in plugin-dns documentation[m
[32m+[m[32m- fd-plugins#3536 Plugins Quota et msg :The field 'Device' contains invalid characters![m
[32m+[m[32m- fd-plugins#3537 Error when I click on reference in EJBCA section[m
[32m+[m[32m- fd-plugins#3555 editing a template inside fai provoke a crash[m
[32m+[m[32m- fd-plugins#3557 Plugin FAI - partition LVM[m
[32m+[m[32m- fd-plugins#3571 Informations for DHCP and DNS are lost after a copy paste[m
[32m+[m[32m- fd-plugins#3592 the ppolicy exemple files should be changed to use a more generic dn[m
[32m+[m[32m- fd-plugins#3621 ejbca my account link is broken[m
[32m+[m[32m- fd-plugins#3626 Export single  entry give an error[m
[32m+[m[32m- fd-plugins#3627 OPSI import should not have an empty filter[m
[32m+[m[32m- fd-plugins#3628 DHCP plugin edit host error[m
[32m+[m[32m- fd-plugins#3637 Applications plugin should not set forceSize parameter[m
[32m+[m[32m- fd-plugins#3638 Error in cyrus template[m
[32m+[m[32m- fd-plugins#3640 Remove a user display an error if we have ppolicy[m
[32m+[m[32m- fd-plugins#3641 Token invalid when we use ppolicy[m
[32m+[m[32m- fd-plugins#3642 FTPStatus attribute must default set at true[m
[32m+[m[32m- fd-plugins#3644 Problem in regex to set release in FAI package list[m
[32m+[m[32m- fd-plugins#3653 Uninitialized string offset: 0 in ldapmanager when importing an ldif[m
[32m+[m[32m- fd-plugins#3663 No image for up/down in FAI profil[m
[32m+[m
[32m+[m[32m## %"FusionDirectory 1.0.8.4" - 2015-01-07[m
[32m+[m
[32m+[m[32m### Added[m
[32m+[m
[32m+[m[32m#### fusiondirectory[m
[32m+[m[32m- fd#3530 the roles in the personal/roles miss an main.inc ![m
[32m+[m[32m- fd#3531 the roles icon is missing in my account[m
[32m+[m
[32m+[m[32m## %"FusionDirectory 1.0.8.3" - 2014-12-15[m
[32m+[m
[32m+[m[32m### Added[m
[32m+[m
[32m+[m[32m#### fusiondirectory[m
[32m+[m[32m- fd#3400 copyright and license is missing for include/functions_debug.inc[m
[32m+[m[32m- fd#3419 FD should allow plugins to act as menu entries providers[m
[32m+[m[32m- fd#3426 cleaning the desktop-management plugin[m
[32m+[m[32m- fd#3429 fusiondirectory-insert-schema should have an option to keep ldifs files[m
[32m+[m[32m- fd#3506 we should have in core a tab in user to be able to assing him roles[m
[32m+[m[32m- fd#3519 Where we add users in sudo plugin their are invisible[m
[32m+[m
[32m+[m[32m#### fusiondirectory-plugins[m
[32m+[m[32m- fd-plugins#3416 create a new web-application plugin[m
[32m+[m[32m- fd-plugins#3427 in some user case we have a need to be able to add roles for users and have those users get rights for applications.[m
[32m+[m[32m- fd-plugins#3450 create a plugin to manage certificates from ejbca[m
[32m+[m[32m- fd-plugins#3472 create a personal plugin to store all kind of personal data[m
[32m+[m[32m- fd-plugins#3496 EJBCA tab for the systems[m
[32m+[m[32m- fd-plugins#3497 EJBCA miss some icons[m
[32m+[m[32m- fd-plugins#3510 Argonaut mirror settings fields have no descriptions[m
[32m+[m
[32m+[m[32m### Changed[m
[32m+[m
[32m+[m[32m#### fusiondirectory[m
[32m+[m[32m- fd#2613 new icon for debug-help aka diagrams has it is the same as ldapmanager[m
[32m+[m[32m- fd#3431 correct all the date and version on the man pages[m
[32m+[m[32m- fd#3441 Inactive tabs could have a different style[m
[32m+[m[32m- fd#3458 making the saving of the samba attributes optional in 1.0.8.x[m
[32m+[m[32m- fd#3468 Hook script with ldap array[m
[32m+[m[32m- fd#3478 Message d'information concernant les propriétes que le mot de passe doit répondre[m
[32m+[m[32m- fd#3493 Department list should be updated after a department creation[m
[32m+[m[32m- fd#3501 the uid id naming mess should be cleaned up[m
[32m+[m[32m- fd#3509 Should french translation use «courriel»?[m
[32m+[m
[32m+[m[32m#### fusiondirectory-plugins[m
[32m+[m[32m- fd-plugins#3401 autofs/admin/autofs/main.inc have a gpl v3 licence and its wrong it should be gpl v2[m
[32m+[m[32m- fd-plugins#3407 plugins are still using obsolete attribute cellspacing on table tags[m
[32m+[m[32m- fd-plugins#3421 memory exhaustion when supann plugin is installed and i try to use the import csv[m
[32m+[m[32m- fd-plugins#3423 cleaning the desktop-management plugin[m
[32m+[m[32m- fd-plugins#3462 update the official files for the supann plugin[m
[32m+[m[32m- fd-plugins#3502 Argonaut is sometimes called «infastructure service» and sometimes «service infrastructure»[m
[32m+[m[32m- fd-plugins#3522 the sudo schema from the sudo-ldap package i using old openldap syntax and must be corrected and included[m
[32m+[m
[32m+[m[32m### Removed[m
[32m+[m
[32m+[m[32m#### fusiondirectory[m
[32m+[m[32m- fd#3396 remove old gosa spec from core-fd.schema[m
[32m+[m[32m- fd#3476 We should remove the ACL tabs from special tabs[m
[32m+[m[32m- fd#3504 FD should stop using the word generic every where[m
[32m+[m[32m- fd#3505 setup_feedback.tpl is a useless file[m
[32m+[m[32m- fd#3517 remove gouvernement mode from fusiondirectory[m
[32m+[m
[32m+[m[32m#### fusiondirectory-plugins[m
[32m+[m[32m- fd-plugins#3393 update-from-1.0.6-to-1.0.7 in plugin-systems[m
[32m+[m[32m- fd-plugins#3402 remove the Authors.rst from the autofs plugin[m
[32m+[m[32m- fd-plugins#3403 remove the glp v3 licence from the autofs plugin[m
[32m+[m[32m- fd-plugins#3404 remove the sieve dir from the mail plugin[m
[32m+[m[32m- fd-plugins#3408 update-from-1.0.6-to-1.0.7 in plugin-mail[m
[32m+[m[32m- fd-plugins#3483 removing leftover of the desktop-management plugin[m
[32m+[m
[32m+[m[32m### Fixed[m
[32m+[m
[32m+[m[32m#### fusiondirectory[m
[32m+[m[32m- fd#3368 Missing png for goServer (systems)[m
[32m+[m[32m- fd#3369 «&amp;» should be used in icon paths[m
[32m+[m[32m- fd#3387 When locking an account with no samba password it adds a samba attribute to it[m
[32m+[m[32m- fd#3399 sometime when refreshing the generic page on a user a get template placeholder in login[m
[32m+[m[32m- fd#3422 when i create a template in the user list i got a missing icon for the new user from template[m
[32m+[m[32m- fd#3430 fusiondirectory-insert-schema man pages is wrong it misses an OPTIONS section[m
[32m+[m[32m- fd#3434 The S placholder to remove whitespace doesnt seems to work[m
[32m+[m[32m- fd#3436 when using apply template after having sucessfully imported user, it wrongly rewrite the users entries[m
[32m+[m[32m- fd#3447 When debug trace level is on, we can’t log in anymore[m
[32m+[m[32m- fd#3470 FusionDirectory in Amazon Linux AMI[m
[32m+[m[32m- fd#3484 The user icon does not show up[m
[32m+[m[32m- fd#3491 User ldap error are wrongly reported[m
[32m+[m[32m- fd#3495 foreignKey on acls is handle several times[m
[32m+[m[32m- fd#3500 Error in user management[m
[32m+[m[32m- fd#3508 There is some french in password recovery[m
[32m+[m[32m- fd#3516 Various problems in strings[m
[32m+[m
[32m+[m[32m#### fusiondirectory-plugins[m
[32m+[m[32m- fd-plugins#3379 The quota tab must have a dependance on the unix tab[m
[32m+[m[32m- fd-plugins#3415 application listing in the desktop plugin doesnt show the icons on the listing[m
[32m+[m[32m- fd-plugins#3433 when importing user and no selection of an field in the csv to be the uid it crash[m
[32m+[m[32m- fd-plugins#3442 dashboard network tab links to dns servers are wrong[m
[32m+[m[32m- fd-plugins#3446 when creating a desktop application it doesnt save it[m
[32m+[m[32m- fd-plugins#3454 Error in the systems plugin documentation[m
[32m+[m[32m- fd-plugins#3459 the nis netgroup is always active[m
[32m+[m[32m- fd-plugins#3485 The user icon does not show up[m
[32m+[m[32m- fd-plugins#3490 GPG plugin have some errors[m
[32m+[m[32m- fd-plugins#3511 The comment for the ssh public key is mandatory[m
[32m+[m[32m- fd-plugins#3512 Application local seems wrong[m
[32m+[m[32m- fd-plugins#3514 Various problems in strings[m
[32m+[m[32m- fd-plugins#3518 remove the french inside the supann plugin[m
[32m+[m[32m- fd-plugins#3520 the default entry in the sudo plugin is not editable anymore[m
[32m+[m
[32m+[m[32m## %"FusionDirectory 1.0.8.2" - 2014-09-25[m
[32m+[m
[32m+[m[32m### Added[m
[32m+[m
[32m+[m[32m#### fusiondirectory[m
[32m+[m[32m- fd#2619 fusiondirectory-insert-schema should have a -c option to continue on error[m
[32m+[m[32m- fd#3335 adding lighttpd support in the doc[m
[32m+[m[32m- fd#3348 add the licence to class_IconTheme.inc[m
[32m+[m[32m- fd#3352 lots of plugins files have no licenses at all[m
[32m+[m
[32m+[m[32m#### fusiondirectory-plugins[m
[32m+[m[32m- fd-plugins#3304 fusiondirectory-shell should have login and password mandatory[m
[32m+[m[32m- fd-plugins#3351 lots of plugins files have no licenses at all[m
[32m+[m
[32m+[m[32m### Changed[m
[32m+[m
[32m+[m[32m#### fusiondirectory[m
[32m+[m[32m- fd#3297 Use relative path for geticon[m
[32m+[m[32m- fd#3356 trust selection mode is broken in group and ogroups[m
[32m+[m
[32m+[m[32m### Removed[m
[32m+[m
[32m+[m[32m#### fusiondirectory-plugins[m
[32m+[m[32m- fd-plugins#1034 remove all dsc form the sources[m
[32m+[m[32m- fd-plugins#3372 remove the trust section from the sudo plugin[m
[32m+[m
[32m+[m[32m### Fixed[m
[32m+[m
[32m+[m[32m#### fusiondirectory[m
[32m+[m[32m- fd#3149 Refresh the Debian install documentation[m
[32m+[m[32m- fd#3332 the documentation on the repos to use for development  is not correct[m
[32m+[m[32m- fd#3359 add user with templates didn't keep group affectation[m
[32m+[m[32m- fd#3361 unable to create a template with posixGroup[m
[32m+[m[32m- fd#3362 Bad aspect for tabs when a dialog is open[m
[32m+[m[32m- fd#3364 we can’t cancel manager selection[m
[32m+[m[32m- fd#3365 there is an error in userManagement template base regexp[m
[32m+[m
[32m+[m[32m#### fusiondirectory-plugins[m
[32m+[m[32m- fd-plugins#3303 debugHelp have problems with tabclasses contaning dashes[m
[32m+[m[32m- fd-plugins#3305 dhcp-fd.schema should be mentionned in dhcp plugin docs instead of dhcp.schema[m
[32m+[m[32m- fd-plugins#3321 Cannot import csv using a template[m
[32m+[m[32m- fd-plugins#3327 primary server in the dns service should not be numeric[m
[32m+[m[32m- fd-plugins#3347 update the doxygen file[m
[32m+[m[32m- fd-plugins#3350 correct the fsf address in licenses[m
[32m+[m[32m- fd-plugins#3363 cannont add Etablissement[m
[32m+[m[32m- fd-plugins#3373 Default action displayed an error[m
[32m+[m
[32m+[m[32m### Security[m
[32m+[m
[32m+[m[32m#### fusiondirectory[m
[32m+[m[32m- fd#3316 possible xss in login screen[m
