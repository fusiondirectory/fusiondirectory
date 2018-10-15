## %"FusionDirectory 1.3" - 2018-09-24

### Added

#### fusiondirectory
- fd#4567 search box suggestions
- fd#4771 extend the search query to email , bis : member objects
- fd#5535 Owner field is missing on group objects
- fd#5626 Add %askme% support for members in groups and roles
- fd#5685 add how to debug fd in the contributing guide
- fd#5697 Add more attributes for times and dates
- fd#5708 Add a way to sort tables
- fd#5734 No ACLs for dashboard
- fd#5738 oGroup not have the ACL for the deployement time frame
- fd#5743 Give more control over snapshot permission
- fd#5745 Special caller* tags should be available in templates as well
- fd#5750 Add the possibility to have a textbox in template to define the manager
- fd#5762 Add hungarian, indonesian, japanese, korean, latvian, turkish, to the available languages
- fd#5826 Add support for date filters
- fd#5834 We cannot use "askme" in BooleanAttribute and SelectAttribute
- fd#5863 adapt fusiondirectory-setup to migrate the supannTypeEntite attribute to contain {SUPANN} prefix
- fd#5885 Add the language Afrikaans (South Africa) (af_ZA) Chinese (Taiwan) (zh_TW)

#### fusiondirectory-plugins
- fd-plugins#1311 Show in system list which system are on/off
- fd-plugins#2347 Two or more MAC address for a system
- fd-plugins#3549 the sudo plugin doesnt allow to save the sudoNotBefore and sudoNotAfter
- fd-plugins#4387 Improve the sorting of services
- fd-plugins#4807 Import LDIF by pasting into textfield
- fd-plugins#5001 config-option "Store final dot in domains" does not effect SOA-checks for powerdns
- fd-plugins#5166 Add a webservice method to add a value to multi-valuated attributes
- fd-plugins#5176 List in applications tab of roles should be sorted alphabetically
- fd-plugins#5430 template placeHolder
- fd-plugins#5518 Allow the list of record inside the dns zone to be sorted by column
- fd-plugins#5543 Enable mail tab for entries having mail attribute
- fd-plugins#5566 System - DHCP Tab - Sort the list Alphabetically
- fd-plugins#5596 Allow to setup OPSI product outside of profiles
- fd-plugins#5675 Add an boolean attribute "agreement signed"
- fd-plugins#5681 The opsi plugin needs the department management dropdown
- fd-plugins#5700 We should be able to forbid deployments outside of a given time frame
- fd-plugins#5706 Add mixed-groups support to sudo plugin
- fd-plugins#5731 Add a tab in the opsi plugin on a system to show log for this computer
- fd-plugins#5733 Add support for social network ORCID in personal
- fd-plugins#5737 Add last date of connection of a client inside the opsi client tab
- fd-plugins#5738 Add opsi stat tab to the dashboard
- fd-plugins#5739 Add a new field into weblink to store an inventory url
- fd-plugins#5746 Add ACL support to dashboard tabs
- fd-plugins#5749 Use target dn as base for ACL checking in deployment queue
- fd-plugins#5756 Extend manager email possibilities in user-reminder
- fd-plugins#5761 Add catchall table to postfix plugin
- fd-plugins#5763 Add template selection into opsi import
- fd-plugins#5764 When importing OPSI hosts, have an option to import the packages associated
- fd-plugins#5787 Add an option to inherit profile from group when OPSI tab is activated
- fd-plugins#5789 Posix plugin should prevent save when automatic group cannot be created
- fd-plugins#5791 Add an option to inherit localboots from group
- fd-plugins#5801 Add other source capabilities for supannTypeEntiteAffectation
- fd-plugins#5816 Audit plugin should have a way to select a range of date to show entries
- fd-plugins#5830 Add domain alias table in postfix plugin
- fd-plugins#5831 Create a new plugin for adding more information related to Supann Structure
- fd-plugins#5845 Add support for FAI 5 to argonaut-fuse
- fd-plugins#5846 Add vfat as filesystem for FAI partition
- fd-plugins#5847 Add a workaround for dracut that use FQDN for hostname

### Changed

#### fusiondirectory
- fd#4467 We should be able to select the phone number from list
- fd#5569 How create a plugin documentation make reference to plugin class
- fd#5570 Restrict member removal through ACLs
- fd#5583 When editing a user, groups and roles tabs shows membership to groups stored outside the configured groups DN
- fd#5590 All plugins icons are missing
- fd#5671 Redesign login screen and password recovery
- fd#5695 Number input fields are too wide
- fd#5704 Use proper pattern matching instead of GOSA_LINE_BREAK hack
- fd#5713 Groups now ignore subgroups for type-checking to know which tabs should appear
- fd#5723 move how_to_contribute page from the wiki to developper documentation
- fd#5725 Minimum PHP version should be bumped to 5.6
- fd#5752 Allow to define multiple LDAP servers for one location
- fd#5761 The language list in user is not sorted
- fd#5802 FileAttribute through webservice should base64 encode binary data to avoid crashes
- fd#5838 Change /location by /Directory in contrib/apache/fusiondirectory-apache.conf
- fd#5883 Update the whole copyright to 2018

#### fusiondirectory-plugins
- fd-plugins#5658 Plugins folder should be reorganized to ease packaging
- fd-plugins#5680 In the argonaut dns plugin we need a dropdow for the zone in the argonaut slave part
- fd-plugins#5696 We should remove the files which moved to dev-tools
- fd-plugins#5702 Making the ip non mandatory in the opsi backend plugin when people don 't manage dns and dhcp with opsi
- fd-plugins#5705 Make mandatory IPs a configuration option in the systems plugins
- fd-plugins#5714 Adapt developers plugin to new plugin convention
- fd-plugins#5722 move how_to_contribute page from the wiki to developper documentation
- fd-plugins#5723 Split posix and ppolicy alert email settings
- fd-plugins#5728 Attribute gotoXdmcpServer is single-value in schema but multi-value in FD
- fd-plugins#5735 Viewing the log from the argonaut queue should not be fai dependent
- fd-plugins#5767 systems-fd.schema classes should allow "description" and "l" fields
- fd-plugins#5772 Adapt code to new snapshots ACL system
- fd-plugins#5811 Missing SUBSTR matching rules for DN foreign keys
- fd-plugins#5814 Adapt code to new Language class
- fd-plugins#5835 add other source capabilities for supannEmpCorp
- fd-plugins#5837 More feature switch should be added to mail methods
- fd-plugins#5848 Matching  mail suffix between PARTAGE and FD
- fd-plugins#5849 fusiondirectory-plugin-renater-partage : gosaMailAlternateAddress domain suffix domain must be checked
- fd-plugins#5855 Change the partage icon to be the new one
- fd-plugins#5861 Update the whole copyright to 2018

### Removed

#### fusiondirectory
- fd#5714 Method ldap::get_attribute should be removed

### Fixed

#### fusiondirectory
- fd#5565 prepare_save should be able to return errors as well
- fd#5571 Crash when I apply a template
- fd#5637 Web applications are not visible
- fd#5657 Groups within Groups Disappear from UI
- fd#5690 Lots of code styling errors
- fd#5722 I cannot use workstation template
- fd#5729 PHP error in class_groupManagement.inc
- fd#5739 Links to objects might fail
- fd#5742 ACL handling for start/stop/restart service is broken
- fd#5747 Deletion of objects should not treat ACL on each tab separately
- fd#5787 member_of_group should be set before attribute loading
- fd#5788 Failed lock attempt gives misleading error (password method does not support locking )
- fd#5798 Tabs are not working on ogroup templates
- fd#5799 When we rename a department foreignKeys on subobjects DN are not updated
- fd#5810 Foreign key on IMAP server is failing
- fd#5811 UID can be changed from the API
- fd#5817 Unique DN creation may fail for audit plugin
- fd#5819 2-letter locales cannot be used (ar, id, ja, ko, lv, nb, pt)
- fd#5820 HTTP header says "Content-Language: ar"
- fd#5837 Update the contrib/apache/fusiondirectory-apache.conf
- fd#5853 Add button for SetAttribute does not get disabled when linearRendering is FALSE
- fd#5855 dashboard crash if we have system plugin but not argonaut plugin in FD 1.3
- fd#5857 Template cannot fill SetAttributes
- fd#5861 We cannot do a snapshot from an element in locality

#### fusiondirectory-plugins
- fd-plugins#5344 We should find a way to translate description of sudo options
- fd-plugins#5638 SSH Plugin - Display bug
- fd-plugins#5655 PHP error in OPSI import
- fd-plugins#5670 Lenteur dans le plugin DNS
- fd-plugins#5691 Lots of styling error in the code
- fd-plugins#5693 update the readme.md with new forge informations
- fd-plugins#5701 Result message displayed when triggerring an action is weird
- fd-plugins#5712 Puppet tab creation should be forbidden if there is no server
- fd-plugins#5721 I cannot use workstation template
- fd-plugins#5727 FusionDirectory display an PHP error when I deactivate "opsi client" tab on a workstation
- fd-plugins#5732 FAI log tab is shown as active even if there is no log
- fd-plugins#5742 OPSI tab shows a failure to fetch logs for templates
- fd-plugins#5743 PHP error in argonaut-queue
- fd-plugins#5748 ACL handling for start/stop/restart service is broken
- fd-plugins#5750 Server copy/paste is broken
- fd-plugins#5752 Renaming a workstation with opsi tab creates another workstation with the new name in OPSI
- fd-plugins#5762 Fatal error: Access level to puppetNode::getDisplayHeaderInfos() must be public (as in class simplePlugin)
- fd-plugins#5765 OPSI tab behaves badly when Mac address is empty
- fd-plugins#5769 DNS fields in record add dialog are greyed out when creating a new system
- fd-plugins#5777 Most POSIX group tabs have no icon set
- fd-plugins#5804 Sympa list alias cn cannot contain '@'
- fd-plugins#5808 Foreign key on mail servers are not working
- fd-plugins#5822 Add a check in the renater partage service class, to check that the class of service is correctly written
- fd-plugins#5825 LDAP error in audit plugin
- fd-plugins#5839 dashboard crash if we have system plugin but not argonaut plugin in FD 1.3
- fd-plugins#5842 Invalid call to strpos in debconfProfileGeneric

## %"FusionDirectory 1.2.2" - 2018-08-16

### Added

#### fusiondirectory-plugins
- fd-plugins#5821 Add LUKS section to FAI disk entry

### Changed

#### fusiondirectory-plugins
- fd-plugins#5836 random_int should be used instead of mt_rand
- fd-plugins#5840 Duplicated value in affiliation file

### Fixed

#### fusiondirectory
- fd#5730 PHP error when I deactivate "opsi client" tab on a workstation
- fd#5789 LDIF import can trigger PHP errors on empty file
- fd#5816 Blacklist is not reset correctly for UserAttribute
- fd#5824 LDIF generation does not respect STARTTLS setting
- fd#5846 Samba groups are not showing in dashboard
- fd#5850 Config insertion LDAP errors from setup are not shown
- fd#5851 Creating a user from a template with a non-existing group crashes
- fd#5864 Name field for countries should be limited to two characters
- fd#5868 HTML is not escaped in departments descriptions

#### fusiondirectory-plugins
- fd-plugins#5719 Locale issues in the fai plugin
- fd-plugins#5741 Reference tab on the OPSI profile is not showing objects reference
- fd-plugins#5810 WebService should not return session cookie
- fd-plugins#5827 Fatal error in opsiProfile when OPSI server is not found in the LDAP
- fd-plugins#5828 supannCodeEntite should allow uppercase as well
- fd-plugins#5832 Copy/pasting of Samba groups fails
- fd-plugins#5833 Error in supann tab when we use a template

### Security

#### fusiondirectory
- fd#5840 Security Vulnerability: Cross Site Request Forgery
- fd#5842 Security: Missing Security Headers
- fd#5843 Security: Insecure Generation of Random Tokens

## %"FusionDirectory 1.2.1" - 2018-05-09

### Added

#### fusiondirectory-plugins
- fd-plugins#5548 Sort entries in ppolicy dashboard

#### fusiondirectory
- fd#5342 SimplePlugin should have a way to say a tab cannot be created for some reason
- fd#5694 Cannot add a group to "users and groups" field in sudo role
- fd#5703 Sort entries in users dashboard
- fd#5746 Using date modifier does not work for unix password expiration

### Changed

#### fusiondirectory-plugins
- fd-plugins#5685 supannRefId on Structure object is too small and mono valued
- fd-plugins#5688 Backport the DNS code from 1.3-dev for speed and usability reason
- fd-plugins#5713 Phones have wrong attribute order
- fd-plugins#5716 DNS Zone edition is too slow
- fd-plugins#5740 Dashboard tabs are in random order

#### fusiondirectory
- fd#5668 Password length on login page should have no limit
- fd#5692 Update the readme.md with correct forge information
- fd#5732 Listing objects should check ACL on attributes
- fd#5737 Dashboard tabs should be ordered
- fd#5808 multibyte functions should be used to truncate strings

### Fixed

#### fusiondirectory-plugins
- fd-plugins#4036 Webservice does not display ACL errors in some cases
- fd-plugins#4348 Fusion inventory should return an error if schema is missing
- fd-plugins#5629 Error message when removing some content from partage tab
- fd-plugins#5672 Mail plugin incompatible with PHP 7.1
- fd-plugins#5673 PHP error on login page when applications plugin is installed and applications in menu set to only allowed
- fd-plugins#5704 [webservice] User  with admin ACL on department can ls objects at root base in WS
- fd-plugins#5707 posixAccount foreignKey is wrong
- fd-plugins#5708 software list and profile management error when using several OPSI Servers
- fd-plugins#5709 DNS Tab - record filtering is broken for new systems
- fd-plugins#5724 SOA serial is rewriten to each reverse even if no modification is made to any record related to these domains
- fd-plugins#5725 Crash when I remove IP from a workstation with a DNS record
- fd-plugins#5729 SOA is not updated in reverse zones if there is no record change
- fd-plugins#5745 PHP error in OPSI product properties
- fd-plugins#5747 Line number in CSV import error messages is incorrect
- fd-plugins#5754 Forward confirmation to manager does not work when extending the account expiration
- fd-plugins#5755 User reminder does not support accents in the message
- fd-plugins#5757 DHCP: Changing mac address in the system's system tab does not update the DHCP entries accordingly
- fd-plugins#5766 Crash when DHCP plugin is not installed when we use OPSI import
- fd-plugins#5768 Renaming DNS zones often results in an Exception being thrown
- fd-plugins#5796 Webservice does not return JSON encode error
- fd-plugins#5798 API returns no error when asking for non existing tab using method getfields()
- fd-plugins#5799 Calling getfields for 'supannAccount' triggers JSON encoding error
- fd-plugins#5813 When we try to put an Google+  id in the personal plugin we get a crash
- fd-plugins#5815 Audit plugin crashes when the author dn is not found in the LDAP

#### fusiondirectory
- fd#5348 LDIF import should support folded comments
- fd#5588 fusiondirectory-setup custom smarty directory forced to lowercase
- fd#5589 SASL missing from Password default hash field in setup wizard
- fd#5701 Management filter is broken when mainAttr is FALSE
- fd#5696 If tab objectClass contains top, tab gets activated by default
- fd#5700 Non-existing classes in pluglist::$info
- fd#5705 Correct contributing remove part about using the templates link
- fd#5707 core-fd.schema still has gosa in description of attributes
- fd#5711 ACL list have a PHP error when there are ACL on objects
- fd#5712 Error text is wrong is some cases
- fd#5724 Error before login on Centos 7
- fd#5726 Log to fd behind an lemonldap::ng portal show debug log instead of main page
- fd#5736 Display Attribute should not be interpreted by smarty
- fd#5741 Foreign keys are broken on ogroups dynamic tabs
- fd#5749 Webservice errors
- fd#5758 Error when renaming a object with single-value main attribute
- fd#5757 In case of LDAP errors during a deletion, a PHP error is triggered
- fd#5759 Unclear "Invalid objectType" errors
- fd#5795 Activating audit for action 'security' causes error at login
- fd#5796 gosaGroupObjects field is incorrect after a member is being moved
- fd#5800 Unclear LDAP errors when DN creation fails
- fd#5807 Attributes should refuse invalid values

### Security

#### fusiondirectory
- fd#5790 Locking fails with multiple SSH Keys
- fd#5794 ACL restrictions not applied when accessing through the JSONRPC API.

## %"FusionDirectory 1.2" - 2017-07-02

### Added

#### fusiondirectory
- fusiondirectory#5581 File renater-partage/admin/groups/renater-partage/class_partageGroup.inc was added

#### fusiondirectory
- fd#5532 We should be able to put groups in groups
- fd#5587 ACL - System templates
- fd#5602 Add array modifiers

#### fusiondirectory-plugins
- fd-plugins#5504 We should be able to launch actions on group of systems
- fd-plugins#5529 Add "Partage" capabilities with SYMPA
- fd-plugins#5534 Add "Partage" capabilities with group
- fd-plugins#5582 Import groups using csv
- fd-plugins#5595 Grab the version of the opsi package in a software-list

### Changed

#### fusiondirectory
- fusiondirectory#5580 class_groupMail.inc was renamed as class_mailGroup.inc
- fusiondirectory#5640 file ldapmanager/addons/ldapmanager/contentcsv.tpl has been removed

#### fusiondirectory
- fd#5545 Unicity is only tested upon the same objectType
- fd#5574 Groups RDN configuration is confusing

#### fusiondirectory-plugins
- fd-plugins#5315 mail methods code should be reviewed and cleaned
- fd-plugins#5341 DHCP tab should be able to load values from template
- fd-plugins#5573 Unicity is only tested upon the same objectType
- fd-plugins#5585 System templates - DNS Tab

### Fixed

#### fusiondirectory
- fd#2586 class_plugin should be reviewed
- fd#4236 FD and IPv6
- fd#5297 User tab / description as text area
- fd#5347 Template types needs their own icon somehow
- fd#5395 Import groups using csv
- fd#5542 Icon for waiting task is not showing
- fd#5568 I have always the browser language, same if I specify another language in FusionDirectory configuration
- fd#5593 Script for hook 'user postmodify' is not executed when password hash=sasl and only the password is changed.

#### fusiondirectory-plugins
- fd-plugins#5340 DHCP postLdapSave should lock the object modified
- fd-plugins#5506 Check that OPSI is fully working on groups
- fd-plugins#5512 Remove DNS tab or machine not remove the record associated
- fd-plugins#5519 template of workstations trigger the creation with the OPSI webservice
- fd-plugins#5520 Schedule action start directly
- fd-plugins#5540 Errors when trying to schedule actions
- fd-plugins#5547 Wrong expiration date shown in ppolicy dashboard
- fd-plugins#5556 %askme% dans les champs date
- fd-plugins#5560 The DNS plugin fails to find the primary server, if it's in another zone
- fd-plugins#5572 Samba home path fields are weirdly checked
- fd-plugins#5636 Slave ldap2zone config should enforce final dot in zone name

## %"FusionDirectory 1.1.1" - 2017-06-06

### Added

#### fusiondirectory-plugins
- fd-plugins#5514 System - DHCP Tab - Preselect DHCP SUBNET according IP address or mask others

### Fixed

#### fusiondirectory
- fd#5511 Error displayed when SASL user created
- fd#5528 PHP error and wrong display in ACL assignments in some cases
- fd#5538 Unable to migrate winstations from 1.0.20 to 1.1
- fd#5554 Trim the certificates path
- fd#5562 Read rights on template let see the users in the root branch

#### fusiondirectory-plugins
- fd-plugins#5550 Unsufficient rights for accessing type 'user'
- fd-plugins#5551 Cannot add a POSIX group to a user
- fd-plugins#5553 Crash error when we remove a posix group that we used in a template
- fd-plugins#5557 Trim the certificates path
- fd-plugins#5564 Automatic primary group is created at the root and not in the branch of the user

## %"FusionDirectory 1.1" - 201-05-18

### Added

#### fusiondirectory
- fd#5428 Add a substitute placeholder
- fd#5431 place holder  : %today%
- fd#5432 Substitute placeholder documentation
- fd#5434 Date place holder should be added to the documentation
- fd#5466 Tabs needs a conflict system
- fd#5470 Add the ability to hide per plugin menu entry (left menu and homepage) to a user or group
- fd#5485 Write process to migrate dhcp from 1.0.16 to 1.1

#### fusiondirectory-plugins
- fd-plugins#2878 we need a way to create systems from the inventory data
- fd-plugins#3147 Fai tab should have a foreignKey on profiles renaming
- fd-plugins#4023 We should be able to create a group with fusiondirectory-shell
- fd-plugins#4358 System templates should be fully functionnal
- fd-plugins#5301 create a plugin to manage the mailbox of renater partage platform
- fd-plugins#5309 DHCP configurations would need foreignKeys on primary and secondary servers
- fd-plugins#5316 Add argonaut client tab to terminals
- fd-plugins#5330 Add to the release note that the format of repository service have changed
- fd-plugins#5382 add a tab on systems to see what sudo roles are added to a specific system
- fd-plugins#5450 add a switch wich decide what to with the mailbox do in case of deleting user
- fd-plugins#5451 see more mailbox information on mail tab
- fd-plugins#5455 Fetch PartageRENATER information
- fd-plugins#5467 LDAP dump could show creation and modification dates

### Changed

#### fusiondirectory
- fd#946 class_plugin should be reviewed
- fd#4893 Plugin saving and hook workflow should be reviewed
- fd#4917 FD should run all the hooks on a given tab
- fd#5041 FIXME legacy code should be removed from callHook method
- fd#5073 Navigation control should only appear if management has baseMode on.
- fd#5091 management should be cleaned
- fd#5235 We should remove from the schemas obsolete stuff
- fd#5311 Reserved words function should be reviewed
- fd#5361 Add an option to split the address in separate fields
- fd#5363 POSIX should be moved to a plugin
- fd#5394 We need a way to test if templates are activated for an objectType
- fd#5479 fusiondirectory-insert-schemas should be based upon ldap-schema-manager

#### fusiondirectory-plugins
- fd-plugins#2419 systemManagement should be redone with simpleManagement
- fd-plugins#2692 Clean systems objectTypes
- fd-plugins#3337 termgroup should be using simplePlugin
- fd-plugins#3614 The gotoMode attribute should be changed to fdSystemLock
- fd-plugins#4556 all the date should be saved in the generalized time match format
- fd-plugins#4677 DHCP plugin should be redone completely with simplePlugin
- fd-plugins#5048 serverService should be based on simpleManagement
- fd-plugins#5312 fdPhone and component situation
- fd-plugins#5331 We should check what is unused in the schemas and mark it OBSOLETE
- fd-plugins#5383 Types list for csv import is too long
- fd-plugins#5384 systems should not require argonaut
- fd-plugins#5441 Allow notify option does not match what it does
- fd-plugins#5463 Workstations tab needs conflict and dependencies

### Removed

#### fusiondirectory
- fd#3998 core functions still using the plugin framework
- fd#5322 LDAP::convert and LDAP::fix functions should be deleted

#### fusiondirectory-plugins
- fd-plugins#3344 the Gohard objectClass should be flagged OBSOLETE and removed from the code
- fd-plugins#5333 kolab2 plugin should be deleted
- fd-plugins#5465 netConfigDNS does not exists anymore

### Fixed

#### fusiondirectory
- fd#3789 tabs and plugin save method should not return anything
- fd#3859 Trustmode in template is automaticaly desactivated
- fd#4029 We should convert all source files to UTF-8
- fd#4171 We should be able to paste a same copied object several times
- fd#4522 setup: __fusiondirectory_autoload tries to load smarty classes
- fd#4688 Use the same message if we set a bad login or a bad password.
- fd#4840 Recursive dependencies should be detected at template saving and forbidden
- fd#4908 Amélioration placement initial du curseur dans les formulaires de saisie
- fd#5016 Fixing wrong FUNCTIONAL filter
- fd#5150 We should not be able of adding users to groups we have no rights on
- fd#5178 fusiondirectory-setup should allow to migrate systems from 1.0.16 to 1.1
- fd#5186 When we edit the logged in user, its info should be updated
- fd#5187 When saving again after an error, it tries to move if dn changed
- fd#5189 The path for icons of template creation are wrong
- fd#5261 Allow for manual configuration of LDAP base
- fd#5328 correct spelling errors found in the manpages
- fd#5334 when migrating from 1.0.16 to 1.0.19 with the new version of --migrate-dhcp there is till a reference not corrected
- fd#5357 Apply template remove sometimes values that are not in the template
- fd#5358 Template et photo
- fd#5364 prelock/postlock hooks cannot be defined
- fd#5367 We should remove from setup migration step obsolete stuff
- fd#5371 PHP error: Only variables should be passed by reference (/usr/share/fusiondirectory/include/functions.inc, line 2538)
- fd#5372 Small error in breezy/style.css
- fd#5380 Bad display of multivaluated attributes in listing columns
- fd#5381 s modifier have problems with multibyte strings
- fd#5385 Several foreignKeys for the same attribute should work
- fd#5398 Cannot remove a role in groups and roles tabs for a template
- fd#5401 size of command field in hook subsystem
- fd#5412 FileTextAreaAttribute modification is broken with default parameters
- fd#5421 Error at first login: «Cannot create locking information in LDAP tree»
- fd#5423 PHP error when listing dcObject
- fd#5433 when trying to change a password i got an error related to audit tool
- fd#5464 Listing objects through objects::ls trigger a first search with scope 1
- fd#5469 Browser specific hacks should be removed
- fd#5474 fusiondirectory-setup should allow to migrate winstations from 1.0.20 to 1.1
- fd#5483 LDAP setup use an OpenLDAP specific syntax
- fd#5487 Applying template to multiple users fills fields from 1st user to the other users
- fd#5509 dashboard : Size limit exceeded

#### fusiondirectory-plugins
- fd-plugins#4516 Inventory matching only works for systems at the root
- fd-plugins#4973 Can't create indirect autofs directory. (slash problem)
- fd-plugins#4993 Shell is broken and field order is random
- fd-plugins#5259 Connection failure between argonaut-server when we search a package
- fd-plugins#5289 user-reminder using ppolicy values
- fd-plugins#5313 Systems dashboard tab needs an update
- fd-plugins#5329 I cannot assign a repository parent in repository service
- fd-plugins#5356 erreur in ldapdump after correcting by hand the dhcpservicedn after migration
- fd-plugins#5365 POSIX should be moved to a plugin
- fd-plugins#5368 Webservice does not return attrs_order for templates (gettemplate method)
- fd-plugins#5400 Erreur fatale : impossible d'initialiser la classe 'argonautEventTypes' - veuillez exécuter 'fusiondirectory-setup --update-cache' pour essayer de régler le problème
- fd-plugins#5411 FAI partition table cannot be saved anymore
- fd-plugins#5413 Some plugins saving are not working properly
- fd-plugins#5414 PHP error in FAI plugin
- fd-plugins#5427 eduPersonAffiliation aren't shown  correctly due to a mix in drop list
- fd-plugins#5429 use partage renater with template
- fd-plugins#5462 DSA: description cannot be set
- fd-plugins#5468 DHCP Plugin ACL role does not apply correctly
- fd-plugins#5471 DHCP configuration enhancement
- fd-plugins#5472 systems aren't showned
- fd-plugins#5476 Service deletion and cancel behaves in a weird way
- fd-plugins#5484 [DNS plugin] Reverse DNS Zones unusable with bind-sdb

## %"FusionDirectory 1.0.20" - 2017-04-04

### Added

#### fusiondirectory
- fd#5282 Documentation for Password Policy Plugin
- fd#5424 Mail alternatif pour la récupération de mot de passe quand supann est activé
- fd#5425 Utiliser le supannAliasLogin dans les messages et les vues de récupération de mot de passe

#### fusiondirectory-plugins
- fd-plugins#5437 argonaut-ldap2zone should be able to get data from a branch
- fd-plugins#5438 we should add the directive check-names into the argonaut-dns service
- fd-plugins#5442 Document fopen activation for argonaut plugin
- fd-plugins#5443 argonaut ldap2zone should be able to create config for dns slave also, but not create the data

### Changed

#### fusiondirectory
- fd#5456 Fine grain ACL control

#### fusiondirectory-plugins
- fd-plugins#5445 reorganize the argonaut dns plugin service

### Fixed

#### fusiondirectory
- fd#5362 PHP errors related to ogroup
- fd#5410 edit conflict warning screen : "read only" button make the same page to reload
- fd#5422 There seems to be a problem with TimeAttribute
- fd#5426 Problems with PHP CAS on centos 7
- fd#5448 Foreign keys from Services does not work

#### fusiondirectory-plugins
- fd-plugins#5370 webservice doesn't provide a way to logout
- fd-plugins#5444 dropdown in systems on the dns tab
- fd-plugins#5446 Error message when trying to update System name in the systems page
- fd-plugins#5452 Error when reloading dhcp service through FD
- fd-plugins#5457 Error modifynig DNS settings in the system's DNS tab

### Security

#### fusiondirectory
- fd#5397 User is able to lock their own account

## %"FusionDirectory 1.0.19" - 2017-01-20

### Fixed

#### fusiondirectory
- fd#5303 When clicking on login from user selection we end up in user edition
- fd#5317 The link sent by password recovery is broken

## %"FusionDirectory 1.0.18" - 2017-01-16

### Added

#### fusiondirectory
- fd#5271 Force encryption type
- fd#5272 Select a default shell
- fd#5293 Webservice: isUserLocked(sid)

#### fusiondirectory-plugins
- fd-plugins#4822 webservice and password recovery
- fd-plugins#4991 we should document all the change in the webservice done recently
- fd-plugins#5240 possibilité de grouper ou trier les applications sur la page d'acceuil
- fd-plugins#5248 [DNS] "Refresh zone file" should be in DNS section too not just in DNS tab of some host where is DNS tab enabled.

### Changed

#### fusiondirectory
- fd#5098 Exception should be reorganized
- fd#5267 Incompatibility between recovery password and user-reminder
- fd#5280 rewrote the pasword recovery with new RDN

#### fusiondirectory-plugins
- fd-plugins#5070 FAI packageSelect class should use simpleSelectManagement
- fd-plugins#5264 Incompatibility between recovery password and user-reminder
- fd-plugins#5294 Exception should be reorganized

### Removed

#### fusiondirectory
- fd#5265 removing RDN leftover of the removal of fax and asterisk plugin

#### fusiondirectory-plugins
- fd-plugins#5266 forgot the remove user reminder config file

### Fixed

#### fusiondirectory
- fd#5258 Cannot modify a package class that is insert with help of fai2ldif
- fd#5260 Check for json support during install.
- fd#5268 Missing check for package php-filter
- fd#5270 List on "ACL assignments management" too short.
- fd#5278 Translation: Plural form of "information"
- fd#5279 Icons in the action column of OrderedArrayAttribute sometimes don’t have enough space
- fd#5283 Path for fusiondirectory-shell
- fd#5298 XML error during installation of FD on debian Wheezy

#### fusiondirectory-plugins
- fd-plugins#4888 DNS record matching should be case insensitive
- fd-plugins#5179 Terminal are missing fields
- fd-plugins#5249 [DHCP] after migration from .16 to .17 there previous configurations are not migrated to new one automatically.
- fd-plugins#5254 Server inside Systems
- fd-plugins#5255 Cannot add a workstation in a dhcp subnet
- fd-plugins#5256 We not need dhcpStatements host-name when we add a host
- fd-plugins#5257 Error in ldap when we will that FAI use all the remain space
- fd-plugins#5295 Filter problem if dhcp object is in a department
- fd-plugins#5296 Name dhcp object differently when we use migration-dhcp
- fd-plugins#5299 When using FD in portal mode, avoid showing applications (which are not web applications)

### Security

#### fusiondirectory
- fd#5252 User with 'editownpwd' or 'editowninfo' role should not be able to lock other accounts
- fd#5292 Malformed data posted to fdPrivateMail through webservice deletes existing data.

## %"FusionDirectory 1.0.17" - 2016-11-06

### Added

#### fusiondirectory
- fd#5230 we need a fusiondirectory-setup --migrate-dhcp

#### fusiondirectory-plugins
- fd-plugins#5193 Add ppolicy tab for DSA accounts
- fd-plugins#5198 merge the dhcp code from 1.1 into 1.0.17
- fd-plugins#5232 we miss a dropdown for failover in the Create new DHCP section
- fd-plugins#5239 Champs pour enregistrer la volonté d'afficher la photo de la personne sur le site web

### Changed

#### fusiondirectory-plugins
- fd-plugins#4953 auto.master should be stored into ldap

### Removed

#### fusiondirectory-plugins
- fd-plugins#5197 removing the ldif for default policy from the ppolicy plugin,
- fd-plugins#5231 we should not be able to remove the global
- fd-plugins#5233 Remove ppolicydefault.ldif and dhcpd.schema from source

### Fixed

#### fusiondirectory
- fd#5184 When a fatal error happens and geticon is not working we see the {t} tag
- fd#5188 Unclear error when geticon.php gets wrong information
- fd#5190 Too many ldap request when loading user list
- fd#5191 Too many ldap request when loading list (snapshots)
- fd#5194 wrong icons / issue with objectType class_groupManagement.inc
- fd#5195 Undefined index: dn / del_lock()
- fd#5196 webservice: object is modified instead of created
- fd#5234 Plugin's configuration are not inserted if they are already installed

#### fusiondirectory-plugins
- fd-plugins#5192 Fusiondirectory ldapmanager plugin fatal error when importing a ldif file
- fd-plugins#5225 due to dhcp modification the network section on system is not inside a section
- fd-plugins#5226 audit showing an error everytime we save an dhcp object
- fd-plugins#5227 moving from branch inside a dhcp entry
- fd-plugins#5228 when trying to copy a dhcp entry is get Missing argument 3 for userinfo::is_copyable(), called in /usr/share/fusiondirectory/include/class_CopyPasteHandler.inc on line 226 and defined" - File: /usr/share/fusiondirectory/include/class_userinfo.
- fd-plugins#5229 when i have a system who is in the dhcp range of my dhcp server, it find and display the right data but don't save it
- fd-plugins#5236 Missed icon in the fusioninventory plugin
- fd-plugins#5241 css error due to sonar cleanup

## %"FusionDirectory 1.0.16" - 2016-10-04

### Added

#### fusiondirectory
- fd#4925 document the autofs plugin
- fd#5096 HTTP header authentication
- fd#5154 Possibility to know who edit the same user when we have a lock entry error
- fd#5157 Possibility to know when was a lock put when we have a conflict
- fd#5162 allow DSA entries to be part of a group
- fd#5163 allow DSA entries to be part of a group - need to a an explanation in the documentation for how to make this fonctionality
- fd#5172 ldap error after upgrade from 1.0.15 release to 1.0.15 fixes (lock object)

#### fusiondirectory-plugins
- fd-plugins#5155 Add an ACL for LDAP tab
- fd-plugins#5156 Add a backend configuration to desactivate the warning popup when we add a user
- fd-plugins#5165 Possible to store sudo role in different OU in the LDAP Tree
- fd-plugins#5173 putting acl on ldap/import export

### Changed

#### fusiondirectory
- fd#5004 Do not require nis schema

### Fixed

#### fusiondirectory
- fd#3587 Edit a template add bad characters
- fd#4808 improve the script error code and return documentation in hooks
- fd#5005 No upper case characters allowed in group-names
- fd#5145 Dashboard should not have references and ldap tabs
- fd#5149 Handling of SSL and URL when behind a proxy
- fd#5151 Create only digit with %r% placeholder not work
- fd#5152 We should not be able of adding users to groups we have no rights on
- fd#5153 Manager that manage a department cannot lock an user

#### fusiondirectory-plugins
- fd-plugins#4997 getfields method returns wrong info in attrs_order
- fd-plugins#5160 opsi import is broken

## %"FusionDirectory 1.0.15" - 2016-08-23

### Added

#### fusiondirectory
- fd#4832 Allow removal of user picture
- fd#4945 Add the possibility to use %askme% for password expiration in template
- fd#5010 HTTP header authentication
- fd#5012 [PRINTERS] old printers are not migrated after upgrade to 1.0.14
- fd#5021 Checkhook should have a way to know if other check errors occured
- fd#5047 add git-shell in default shell list

#### fusiondirectory-plugins
- fd-plugins#4991 we should document all the change in the webservice done recently
- fd-plugins#5025 Manage pwdPolicyChecker objectClass
- fd-plugins#5051 add a url text field for the linkedin account url
- fd-plugins#5056 Need to edit class_argonautActions.inc for custom modules
- fd-plugins#5088 Add samba shares update action to the list

### Changed

#### fusiondirectory
- fd#5052 password storage in firefox provoque an issue in the password field of the user tab in FusionDirectory
- fd#5057 check_schema should be reviewed
- fd#5089 Attribute dob of fusiondirectory schema conflicts with evolution schema

#### fusiondirectory-plugins
- fd-plugins#5031 Icon for LDAP should be moved to core
- fd-plugins#5043 GPG server info cannot be edited anymore
- fd-plugins#5068 PHP4 constructor in FAI plugin
- fd-plugins#5069 PHP4 constructor in mail plugin
- fd-plugins#5090 Attribute dob of fusiondirectory schema conflicts with evolution schema

### Removed

#### fusiondirectory-plugins
- fd-plugins#5045 Apache plugin should be removed

### Fixed

#### fusiondirectory
- fd#5003 Error on mixed groups when nis schema not present
- fd#5022 Random password does not work when we use a template in read only
- fd#5023 Icon for section account is not found
- fd#5040 Uppercase login triggers php notice
- fd#5049 probleme de traduction dans le systeme de reinitialisation des mots de passe
- fd#5050 mauvais encodage dans le message de demande de reinitialisation du mot de passe
- fd#5058 notice on 1.0.14-fixes
- fd#5071 Fatal error in FAI plugin
- fd#5074 Error to add ACL
- fd#5092 FD should not interfere with form handling

#### fusiondirectory-plugins
- fd-plugins#5020 PHP errors in DHCP plugin with PHP7
- fd-plugins#5026 CSV import bug ?
- fd-plugins#5032 check that the information for main menu is right in FusionDirectory documentation
- fd-plugins#5044 Use simpleManagement in GPG plugin
- fd-plugins#5055 Argonaut actions launching is broken
- fd-plugins#5075 Error to add ACL

## %"FusionDirectory 1.0.14" - 2016-07-10

### Added

#### fusiondirectory
- fd#4796 documentation for centos 6
- fd#4831 Allow setting server name for password recovery feature
- fd#4913 adding the eps and png of the FusionDirectory logo to the sources
- fd#4924 adding the language Finnish (Finland) (fi_FI)
- fd#4970 templates have no acls
- fd#4982 CSV import base edition is broken

#### fusiondirectory-plugins
- fd-plugins#4142 Local quota
- fd-plugins#4955 add two boolean attributes to community organization

### Changed

#### fusiondirectory
- fd#4897 the breezy theme should be the default one in 1.0.14
- fd#4915 refaire la favicon de FusionDirectory

#### fusiondirectory-plugins
- fd-plugins#4894 printGeneric code is too old and ugly and 1.1 code should be used instead
- fd-plugins#4934 the default theme should be renomed legacy or kde3
- fd-plugins#4937 the breezy theme should be the default one in 1.0.14
- fd-plugins#4954 Regression: "Generic IMAP Server" (fdImapServer) was removed

### Removed

#### fusiondirectory-plugins
- fd-plugins#4927 fdCommunityMembershipType should no be used/displayed in community projects dep.
- fd-plugins#4946 the game plugin should be removed as its not longer usefull

### Fixed

#### fusiondirectory
- fd#4882 Session lifetime...
- fd#4890 Configuration saving is failing in setup
- fd#4896 strings in recovery.tpl are not put in .po file
- fd#4904 changement de mot de passe reste forcé après mise à jour du mot de passe
- fd#4907 dans le template user, il ne mémorise pas la case forcer le changement mdp à la 1ere connexion
- fd#4910 Greyed out icon are the same as normal one with breezy theme
- fd#4912 erreurs php dans /var/log/messages (class_IconTheme.inc)
- fd#4916 Les liens sont en bleus à beaucoup d’endroit où cela semble inadapté
- fd#4918 Clean tpl files
- fd#4920 plugins where there is still some images not in theme
- fd#4921 update timeout colors
- fd#4923 uid not passed to the mail-method cyrus
- fd#4972 ACL assignment is broken
- fd#4974 LDIF import crash on value 0
- fd#4980 Icon status/dialog-warning missing in size 48 in breezy
- fd#4981 trying to put an ACL assignments on an non existing branch crashes

#### fusiondirectory-plugins
- fd-plugins#4766 testing the autofs plugin
- fd-plugins#4772 Autofs - Taille de champ "entrée automount" trop petite
- fd-plugins#4773 Autofs - Vu liste n'affiche pas l'information principale pour les "Répertoires"
- fd-plugins#4874 plugins where there is still some images not in theme
- fd-plugins#4876 Can't edit system: "Workstation"
- fd-plugins#4877 Can't edit Dns Zone.
- fd-plugins#4878 Can't create "Network devices"
- fd-plugins#4879 Can't add "printer" on Workstation
- fd-plugins#4880 Remote Desktop stay greyed out.
- fd-plugins#4881 Is possible to modify url link?
- fd-plugins#4884 Case sensitive and DNS
- fd-plugins#4922 old option option still present in cyrus mail method
- fd-plugins#4943 when clicking on the left menu on the gpg entry FusionDirectory crash
- fd-plugins#4944 when clicking on the left menu on the opsi import entry FusionDirectory crash
- fd-plugins#4956 errror when suppressing or adding some service
- fd-plugins#4971 Some services icons are missing
- fd-plugins#4992 PHP error "deprecated call to old tabclass constructor"
- fd-plugins#4994 Shell is broken

### Security

#### fusiondirectory
- fd#4875 Hooks can lead to arbitrary code execution

## %"FusionDirectory 1.0.13" - 2016-06-13

### Added

#### fusiondirectory
- fd#4774 Add an LDAP Audit system
- fd#4803 "Member object"  suggestions
- fd#4853 get the value of the connect user in a variable to be passed to scripts hooks

#### fusiondirectory-plugins
- fd-plugins#4775 Create an expiration alert mail script
- fd-plugins#4783 Use of templates through the webservice
- fd-plugins#4854 adding "lock/unlock entry" function to the webservice

### Changed

#### fusiondirectory-plugins
- fd-plugins#4787 Mail server should not be mandatory in alias plugin
- fd-plugins#4790 make functions names consistent
- fd-plugins#4799 creating an object with several activated tabs in one go

### Removed

#### fusiondirectory
- fd#4757 padlock next to user dn in edition screen is confusing
- fd#4826 File docs/INSTALL must be updated or deleted

### Fixed

#### fusiondirectory
- fd#4759 disabled account can still reset password ?
- fd#4794 sambaNTpassword is not updated when password is changed through «My account» menu
- fd#4806 Importing ldif file with comments generates error
- fd#4821 password recovery, email field focus
- fd#4823 Imagick is not detected during the setup
- fd#4824 Error when I will create fd-admin in the setup
- fd#4828 FusionDirectory does not work with Smarty 3.1.29
- fd#4863 top header of the login page keep the green even when the breezy them is activated

#### fusiondirectory-plugins
- fd-plugins#4763 Translation mishaps in dovecot plugin
- fd-plugins#4776 unwanted debug in jsonrpc.php
- fd-plugins#4782 Systems snapshots are broken
- fd-plugins#4788 'Incorrect response id (request id: 1, response id: )'
- fd-plugins#4789 Webservice setfields method needs documentation
- fd-plugins#4791 setfields() : values structure and fields names
- fd-plugins#4855 PHP error about DNSenabled
- fd-plugins#4856 Copy past test of server
- fd-plugins#4857 When mail can be used to login, it should be unique

### Security

#### fusiondirectory
- fd#4764 Fusiondirectory exposes bindpw on error

## %"FusionDirectory 1.0.12" - 2016-05-04

### Added

#### fusiondirectory
- fd#4612 Graphical bugs if prototype is not installed
- fd#4687 ACL assignments can’t give rights to groups.
- fd#4722 Use Nginx instead of Apache2
- fd#4735 nginx + php-fpm with fusiondirectory
- fd#4738 extend the search query to email

#### fusiondirectory-plugins
- fd-plugins#4742 ppolicy: description cannot be set
- fd-plugins#4743 ppolicy: user interface improvements

### Changed

#### fusiondirectory
- fd#4678 group : member objects list
- fd#4746 Error when saving user if objectClass person is missing.

#### fusiondirectory-plugins
- fd-plugins#4689 we should be able to create mail address without having a server and a service referenced
- fd-plugins#4699 move postfix service in his own plugin
- fd-plugins#4706 move the spamassasin service to his own plugin
- fd-plugins#4721 move the spam options for the mail user tab to a spamassasin user tab

### Removed

#### fusiondirectory
- fd#4713 remove the antivirus service icons
- fd#4737 we should remove data['SERVERS']['IMAP'] from config
- fd#4755 user template: password should show only one field

#### fusiondirectory-plugins
- fd-plugins#4692 remove the antivirus service
- fd-plugins#4714 leftover in services-fd.schema of the removal of the asterisk plugin
- fd-plugins#4739 remove the option Use custom sieve script (disables all Mail options!)

### Fixed

#### fusiondirectory
- fd#4754 user template : %r|% does not work in password

#### fusiondirectory-plugins
- fd-plugins#4658 csv import with empty field
- fd-plugins#4740 in the cyrus service the hostname, port should be mandatory
- fd-plugins#4741 in the dovecot service the hostname, port should be mandatory

## %"FusionDirectory 1.0.11" - 2016-04-07

### Added

#### fusiondirectory
- fd#4659 when creating a user via  template or csv import, mandatory , alreay defined fields have to be re-inputed even if already defined

#### fusiondirectory-plugins
- fd-plugins#4632 Webservice needs a better method for modification
- fd-plugins#4660 csv import lacks an option or help on what to do with the first line

### Changed

#### fusiondirectory-plugins                                                                                                                                                                                                     
- fd-plugins#4657 community organization membership type                                                                                                                                                                         
                                                                                                                                                                                                                                 
### Removed                                                                                                                                                                                                                      
                                                                                                                                                                                                                                 
#### fusiondirectory                                                                                                                                                                                                             
- fd#4621 Use of mcrypt should be removed                                                                                                                                                                                        
- fd#4652 The weird _copy fallback should be removed                                                                                                                                                                             
                                                                                                                                                                                                                                 
#### fusiondirectory-plugins                                                                                                                                                                                                     
- fd-plugins#4654 DHCP seems to use inexistant method getCn                                                                                                                                                                      
                                                                                                                                                                                                                                 
### Fixed                                                                                                                                                                                                                        
                                                                                                                                                                                                                                 
#### fusiondirectory                                                                                                                                                                                                             
- fd#4633 change requirement to php 5.4 for centos
- fd#4634 DNS PTR migration does not work
- fd#4638 UI issue, entry disappear in 'Base' dropdown
- fd#4639 depending on FD language setting, the calendar doesn't show up on icon click
- fd#4640 strange error when following org's link from user "references" tab
- fd#4653 Having a wrong default location should trigger a fatal error
- fd#4655 The edit anyway button does not work in my account menu
- fd#4673 ImageAttribute are broken
- fd#4674 translation issue  : department is service in french not département

#### fusiondirectory-plugins
- fd-plugins#4391 Choices order seems random in shell
- fd-plugins#4630 DNS adding reverse zone issue (tested on FD 1.0.10) in class B and class A
- fd-plugins#4635 [DNS] Reverse zone association breaks when there are several A record for the same domain.
- fd-plugins#4636 [SYSTEMS] When add Windows Workstation (winStation) and enable DHCP it will not be enabled when edit again
- fd-plugins#4637 DNS tab won’t work on winstations
- fd-plugins#4676 Edit IDs problems related to our selenium test

## %"FusionDirectory 1.0.10" - 2016-03-20

### Added

#### fusiondirectory
- fd#4492 "login" name in group add page
- fd#4515 fusiondirectory-setup need an option to migrate the dns 1.0.9.x version to 1.0.10
- fd#4529 The category in department and organization should be a dropdown filled from the configuration backend
- fd#4530 in the department and organization we need to add the labeledUri
- fd#4531 The co (country friendlyName) in department and organization should be added and be dropdown filled from the configuration backend
- fd#4539 add mobile phone to the user page
- fd#4543 Add an option to fusiondirectory-setup to see the uid that are duplicated
- fd#4561 be able to select all user from ou=people,dc=opensides,dc=be but be confined to your branch for all the rest
- fd#4562 add http auth
- fd#4564 Add a C modifier for counting an array
- fd#4565 Fixedbutton themes should replace the default one

#### fusiondirectory-plugins
- fd-plugins#4484 allow ldap ipv6 uris
- fd-plugins#4496 DNS plugin should support RFC 2317 for reverse zones
- fd-plugins#4504 Support for split horizon should be added
- fd-plugins#4511 Add a button to launch argonaut-ldap2zone from FusionDirectory
- fd-plugins#4525 we should have attribute for start and end date into the personal-fd.schema and add it to the personal schema
- fd-plugins#4526 add "co" attribute into personal-fd.schema and into personal plugin
- fd-plugins#4527 create a community plugin to manage community aka organization and projects
- fd-plugins#4528 tab community on the user
- fd-plugins#4532 the personal plugin should have a field where we can put the badge number
- fd-plugins#4533 make a subcontracting plugin
- fd-plugins#4580 Newsletter plugin

### Changed

#### fusiondirectory
- fd#4480 LDAP::convert and LDAP::fix functions are causing problems
- fd#4518 Plugin deletion contains some useless ldap operation
- fd#4519 ldap::rmdir_recursive is not optimal
- fd#4524 To be able to add the organization URI into the User part of the user, the uri from Personal contact information should be moved to personal plugin
- fd#4610 postalAddress (and homepostalAddress) content should stick to the RFC
- fd#4611 setRequired should be public

#### fusiondirectory-plugins
- fd-plugins#1217 DNS service must be redone
- fd-plugins#4468 Error when I try to move a system with the DNS tab activate
- fd-plugins#4592 its best if we use the /html/themes everywher in place of /html/images evertime is possible
- fd-plugins#4620 postalAddress (and homepostalAddress) content should stick to the RFC

### Removed

#### fusiondirectory
- fd#4517 Some obsolete objectClass are still searched for in config

#### fusiondirectory-plugins
- fd-plugins#4490 nagios.tpl file is not used anymore
- fd-plugins#4520 Syslog server field should be removed
- fd-plugins#4521 LDAP server field should be removed (gotoLdapField)
- fd-plugins#4523 ntp server should be removed
- fd-plugins#4576 create tab for the plugin community onto organization and department

### Fixed

#### fusiondirectory
- fd#4476 rewrite the part about fusiondirectory core in the documentation
- fd#4477 rewrite the configuration password recovery page as this as been integrated into the configuration in core
- fd#4478 Using %askme% in User id and group id in templates does not work
- fd#4482 Debug log prevent connection in some cases
- fd#4486 possible documentation tweaks
- fd#4497 The order of user id and group id are inverted
- fd#4498 Bad error message when we will use a GID with a non-existent group
- fd#4505 fusiondirectory-setup --migrate-users not work when the users don't have all the samba attributs
- fd#4508 second email and first email are inverted in the password recovery configuration page
- fd#4544 Possibility to see the uid that are duplicate when we install FusionDirectory
- fd#4557 when a user is on top people branch and get an acl in subtree for a lower branch i get " The supplied base is not valid and has been reset to the previous value! "
- fd#4569 Groups containing unknown type objects or deleted objects does not appear
- fd#4600 'base' sticky or not ?
- fd#4606 the login column is missing in the groupOfname user selection list
- fd#4613 Error about sambaUnixIdPool is unclear

#### fusiondirectory-plugins
- fd-plugins#4495 Use ldap_escape in compute_dn
- fd-plugins#4512 when using cyrus in unix hierarchy mode and creating a user and activating mail tab at the same time, the mailbox is not created
- fd-plugins#4514 It takes too long to load the reference tab for a DNS zone
- fd-plugins#4540 Problème avec les vcf (vcards)
- fd-plugins#4555 the terminal plugin has an error with the latest ntp removal fix
- fd-plugins#4563 DNS zone serial is not updated when record are changed through the DNS tab of a system
- fd-plugins#4575 Error with samba plugin and sambantpassword attribute
- fd-plugins#4591 It’s impossible to delete root records in DNS zones
- fd-plugins#4622 DNS record should be sorted

## %"FusionDirectory 1.0.9.3" - 2016-02-06

### Added

#### fusiondirectory
- fd#4459 Desactivate other authentification method from configuration file

### Changed

#### fusiondirectory
- fd#4485 'fusiondirectory-insert-schema -m' should check if the specificed file exists

#### fusiondirectory-plugins
- fd-plugins#4456 Can't find a way to specify a '/' in distribution or media name in the repository plugin
- fd-plugins#4465 remove from the service-fd.schema objectClass no longer used

### Removed

#### fusiondirectory
- fd#4479 In primary group in unix tab there is a «None» option
- fd#4481 «Enable primary group filter» option is obsolete

#### fusiondirectory-plugins
- fd-plugins#4463 There are still traces of the rsyslog plugin in the schema and config class

### Fixed

#### fusiondirectory
- fd#4444 Debug log prevent connection in some cases
- fd#4452 Adding a user to a group using "Groups and roles" tab creates some PHP errors
- fd#4464 There are still traces of the rsyslog plugin in the schema and config class
- fd#4466 fixing deprecated warnings in php7

#### fusiondirectory-plugins
- fd-plugins#4457 FusionDirectory don't find the dns server if he is in a department

### Security

#### fusiondirectory
- fd#4473 Locked users can connect using SSH keys

#### fusiondirectory-plugins
- fd-plugins#4385 Locked users can connect using SSH keys

## %"FusionDirectory 1.0.9.2" - 2016-01-10

### Added

#### fusiondirectory
- fd#4001 passwordProposalHook
- fd#4216 the setup in the ldap inspection step need a refresh button
- fd#4217 the reset password system should look for supannMailPerso if the supann  plugin is installed
- fd#4221 developer doc on documentation.fusiondirectory.org should have an explanation of the is_this_account system
- fd#4239 ReadOnlyLDAPAttribute should be moved to class_attribute.inc
- fd#4280 passwordProposalHook documentation should be written
- fd#4288 the documentation about automation should be added

#### fusiondirectory-plugins
- fd-plugins#4220 Add a function to list the tab that are activate on an object in fusiondirectory-shell
- fd-plugins#4334 the url in the webservice-shell should have the jsonrpc.php added automatically
- fd-plugins#4353 Error when I try to grab centos packages with an argonaut-server on centos
- fd-plugins#4409 the ssh plugin doesnt support ecdsa-sha2-nistp256 ssh public key format

### Changed

#### fusiondirectory
- fd#2166 Semantic of plDescription and plShortname
- fd#4219 allow FusionDirectory to be integrated into a cas infrastructure
- fd#4343 Underscores in departement/domain component names not allowed
- fd#4390 bump all copyright to 20xx-2016

#### fusiondirectory-plugins
- fd-plugins#4111 Clean FAI support daemon method
- fd-plugins#4227 Change default block size value in 1024
- fd-plugins#4238 ReadOnlyLDAPAttribute should be moved to class_attribute.inc
- fd-plugins#4292 Migrate last FAI classes to simplePlugin
- fd-plugins#4298 the usage of the fusiondirectory-shell doesnt explain the various options for connecting
- fd-plugins#4364 Clean up the DaemonEvent mess
- fd-plugins#4388 removed old gosa copyright in new argonaut code
- fd-plugins#4389 bump all copyright to 20xx-2016

### Removed

#### fusiondirectory
- fd#4281 PHP error about wrong acl when I edit the configuration
- fd#4347 Remove config search method
- fd#4367 We should delete the databaseManagement class

#### fusiondirectory-plugins
- fd-plugins#4109 The fai tab should not contain kernel options
- fd-plugins#4365 We should delete the rsyslog plugin
- fd-plugins#4394 Remove the mail method Cyrus Sendmail
- fd-plugins#4419 remove the class_BootKernelAttribute.inc
- fd-plugins#4425 remove fix_munged from the samba plugin

### Fixed

#### fusiondirectory
- fd#4231 difference in provided ldap server in debian and Centos
- fd#4253 PHP error when I go at dashboard
- fd#4254 we can no longer select a windows workstation in the user samba tab systeme trust
- fd#4261 It appears that prehooks not work with password recovery
- fd#4345 password recovery show an error when activating recovery password
- fd#4378 user deleted when we change its base
- fd#4380 when migrating users with fusiondirectory-setup –migrate-users it cannot migrate user with samba attributes

#### fusiondirectory-plugins
- fd-plugins#3880 mailbox deletion through sieve and cyrus does not work
- fd-plugins#4067 Argonaut Server documentation should mention «Get packages informations» option
- fd-plugins#4200 I must tick "Search in subtrees" to see the elements in repository management
- fd-plugins#4209 we can no longer select a windows workstation in the user samba tab systeme trust
- fd-plugins#4225 the reset password system should look for supannMailPerso if the supann  plugin is installed
- fd-plugins#4241 Problem when I try to save an already existing supann entry - dropdown dependancy not already filled
- fd-plugins#4344 when saving the configuration with the application plugin installed
- fd-plugins#4346 CSV import is broken
- fd-plugins#4379 can't change informations in "Network Settings" in a network component object in systems
- fd-plugins#4382 SSH plugin: error while adding keys with space(s) in comment
- fd-plugins#4392 Error when I open a system from the inventory menu
- fd-plugins#4395 PHP errors when I open inventory tab from a system

## %"FusionDirectory 1.0.9.1" - 2015-10-03

### Added

#### fusiondirectory
- fd#4149 Removal of the dashboard schema with -e is not in the migration documentation
- fd#4154 add support for displayName attribute
- fd#4185 The configuration is lost after upgrading to 1.0.9.1
- fd#4186 The 1.0.9.x version breaks my password hooks
- fd#4187 Documentation for new password use
- fd#4190 fusiondirectory-setup should be improved
- fd#4197 When we upgrade from 1.0.8.9 at 1.0.9 the old schemas are not removed
- fd#4208 list of group membership in user when in mixed mode 1.0.9x is not showing the same info as in 1.0.8.9

### Changed

#### fusiondirectory
- fd#4168 Underscores in group names not allowed anymore
- fd#4173 When mixed group plugin is installed group and ogroup tabs should be merged
- fd#4196 roles should no allow space in their name

#### fusiondirectory-plugins
- fd-plugins#4180 on Droplist, put entries in alphetical order

### Removed

#### fusiondirectory
- fd#4136 we should remove the samba.schema from our contrib directory in core source
- fd#4193 Remove gosaAccount with fusiondirectory-setup --migrate-users

#### fusiondirectory-plugins
- fd-plugins#4139 Remove old documentation

### Fixed

#### fusiondirectory
- fd#4158 Fatal error after role creation
- fd#4172 Filter for ogroups should be on groupOfNames
- fd#4179 st field seems to be reading correctling but not setting correctly in a template
- fd#4184 We missed a foreignkey in system trust in unix tab for user
- fd#4191 Error in the fusiondirectory-setup
- fd#4194 list of users membership in group when in mixed mode 1.0.9x is not showing the same info as in 1.0.8.9
- fd#4195 when going to the page groups and roles with mixedgroup plugin installed
- fd#4206 when a mixed group posixGroup and groupOfNames without the extra fd objectClass and attributes is opened then saved it tell me a group with the same name already exist

#### fusiondirectory-plugins
- fd-plugins#4169 Uncaught exception 'NonExistingObjectTypeException' with message 'Non-existing type "group"
- fd-plugins#4183 option host-name with winstations results in error
- fd-plugins#4199 the logon hours page is not correctly constructed

## %"FusionDirectory 1.0.9" - 2015-09-24

### Added

#### fusiondirectory
- fd#2516 Having a way of generating objectTypes graphs
- fd#3064 Migrate fai to simplePlugin
- fd#3158 FusionDirectory could have an alternate theme
- fd#3233 We should use freedesktop Icon Theme Specification
- fd#3247 Plugins needs to be able to know which objectType they are opened as
- fd#3594 We should be able to use roles for ACL assignements
- fd#3730 Ppolicy plugin
- fd#3771 Add /sbin/nologin in shells
- fd#3780 We should migrate DNS and DHCP services root objects
- fd#3787 template-fd.schema should be in the default schemas in fusiondirectory-insert-schema
- fd#3804 Documentation for using role in acl assignement
- fd#3813 Ppolicy plugin should check password history
- fd#3845 Error when we add a posix group
- fd#3857 We should be able to apply templates
- fd#3905 Icons of actif tabs not appear on template
- fd#3931 We need a plugin for support of ugly groups
- fd#3959 Password force default hash does not work
- fd#4008 edit locking should have a default value selected during web setup
- fd#4012 Argonaut server should support HTTPS
- fd#4040 add spanish colombian to FD
- fd#4143 the template management has completely changed in 1.0.9 so we need a new documentation about this
- fd#4152 Add an example of -o option to man page of fusiondirectory-insert-schema

#### fusiondirectory-plugins
- fd-plugins#955 write https support for PHP JSONRPC
- fd-plugins#2678 sudoOption should allow to negate
- fd-plugins#2894 More plugins should use foreignKeys
- fd-plugins#2948 we should add write features to the webservice
- fd-plugins#3258 Paste SSH public key directly in FD interface
- fd-plugins#3733 Systems classes needs compute_dn methods
- fd-plugins#3748 Set a default secure_path when we activate it
- fd-plugins#3777 remove the obligation to have an gosaAccount on user entries to be displayed by FusionDirectory
- fd-plugins#3844 Add title option on the tr
- fd-plugins#3922 We need a plugin for support of ugly groups
- fd-plugins#3929 Group synchronization Samba / UNIX
- fd-plugins#3966 Argonaut server should support HTTPS
- fd-plugins#3980 move phones out the asterisk plugin to the system plugins
- fd-plugins#4000 some entries in the supann plugin don't have english explanation next to the attribute name
- fd-plugins#4016 Webservice must respect ACLs
- fd-plugins#4022 We should be able to create a user with fusiondirectory-shell
- fd-plugins#4042 We missed a method to delete objects with fusiondirectory-shell
- fd-plugins#4047 creating an argonaut-fai-monitor service
- fd-plugins#4062 Specify POSIX group in a message
- fd-plugins#4145 Systems plugin needs to include obsolete asterisk attributes

### Changed

#### fusiondirectory
- fd#1887 Template system should be redone
- fd#1955 unique is checked only in the object base
- fd#2004 Object groups should be handled automatically
- fd#2407 Migrate class_user to simple-plugin
- fd#2602 Improve debug-help plugin
- fd#2608 group should use simplePlugin
- fd#2622 Select classes should be cleaned and regrouped
- fd#2624 ogroup should use simplePlugin
- fd#2919 Setup should be rewritten
- fd#3078 Clean ArrayAttribute hierarchy
- fd#3248 Plugins needs to be able to know which objectType they are opened as
- fd#3250 unique is checked only in the object base
- fd#3299 Use relative path for geticon
- fd#3317 LDAP reorganization
- fd#3334 userManagement should be using simpleManagement
- fd#3383 Password change and hooks should be cleaned
- fd#3384 We need to remove manual handling of foreignKeys
- fd#3452 remove the obligation to have an gosaAccount on user entries to be displayed by FusionDirectory
- fd#3471 move the objectgroup plugin inside the group plugin
- fd#3602 password recovery config should be merged with core configuration
- fd#3636 Main menu should handle better icon sizes
- fd#3718 groupOfNames and rfc2307bis situation must be cleaned
- fd#3848 generic user view should be reorganized
- fd#3850 the DESC of each attribute and object should describe exactly the purpose of it
- fd#3851 it seems the gosaCacheEntry objectClass is no longer used
- fd#3907 Rework CN and DN related options
- fd#3921 User templates should support roles
- fd#4051 Mac addresses should always be in lowercase
- fd#4100 The logout screen should be redone (session expiry)
- fd#4146 check the copyright date on licence in each file

#### fusiondirectory-plugins
- fd-plugins#2509 LDAP manager should be using simplePlugin
- fd-plugins#2629 Object groups should be handled automatically
- fd-plugins#2642 Select classes should be cleaned and regrouped
- fd-plugins#2650 ogroup should use simplePlugin
- fd-plugins#3063 Migrate fai to simplePlugin
- fd-plugins#3272 All systems object should use simpleTabs
- fd-plugins#3343 put the obsolete word into the system-fd.schema each time we see deprecated
- fd-plugins#3386 We need to remove manual handling of foreignKeys
- fd-plugins#3388 Samba account should handle samba password
- fd-plugins#3418 ButtonAttribute API has changed
- fd-plugins#3451 Some fai code should be made generic
- fd-plugins#3503 Argonaut is sometimes called «infastructure service» and sometimes «service infrastructure»
- fd-plugins#3593 We should migrate DNS and DHCP services root objects
- fd-plugins#3743 In FAI plugin base propositions should contain distribution
- fd-plugins#3862 developement documentation should be updated for icons situation
- fd-plugins#4059 Manage argonaut-fuse with FusionDirectory and remove serviceRepository from the actions
- fd-plugins#4075 Allow select Supann Role by choosing in several base
- fd-plugins#4102 Kolab plugin should specify «Kolab 2»

### Removed

#### fusiondirectory
- fd#3222 Clean dead code
- fd#3231 Remove useless snashot fields from config ldap schema
- fd#3722 We should remove the «enable copy&paste» option
- fd#3741 Error when I do fusiondirectory-insert-schema
- fd#3755 fusiondirectory.conf attribute configVersion is unused and should be removed
- fd#3839 cleanup the icons situation
- fd#3846 template are in their own branch now but code still reference gosaUserTemplate objectClass who is deprecated
- fd#3849 we should remove (v2.6.1) from the schema descriptions
- fd#3853 Remove deprecated objectClass and attributes of 1.0.8.7
- fd#3858 Delete CUPS service references and goService
- fd#3914 We should remove the class tabs
- fd#3947 FD should stop using the word generic every where
- fd#3949 We should get rid of get_post function
- fd#3990 removing the test in fusiondirectory web setup for the database library
- fd#4002 password.tpl is not used anymore it should be deleted
- fd#4014 function missing in personal/generic/class_user.inc
- fd#4056 We should remove the Automatic logins option
- fd#4114 remove slapd.conf from contrib/openldap as no one use slapd.conf anymore and there is error in it

#### fusiondirectory-plugins
- fd-plugins#700 Test the uw-imap plugin
- fd-plugins#3345 remove the uw-imap plugin from fusiondirectory sources
- fd-plugins#3694 openstack-compute is obsolete and should be deleted
- fd-plugins#3840 cleanup the icons situation
- fd-plugins#3847 template are in their own branch now but code still reference gosaUserTemplate objectClass who is deprecated
- fd-plugins#3852 we should remove (v2.6.1) from the schema descriptions
- fd-plugins#3856 Delete CUPS service
- fd-plugins#3916 We should remove the class tabs
- fd-plugins#3981 remove the asterisk plugin from 1.0.9
- fd-plugins#3992 remove the fax plugin
- fd-plugins#4034 Remove the FIXME in FAI about priority
- fd-plugins#4066 Weblink refers to IPMI
- fd-plugins#4068 password.tpl is not used anymore it should be deleted
- fd-plugins#4107 the autodect button for the mac address should be removed

### Fixed

#### fusiondirectory
- fd#2595 The code should follow code guidelines
- fd#3237 System trust in Unix tab is broken
- fd#3342 Welcome icon is broken in 1.0.9
- fd#3355 weird things happens when moving error pop-ups
- fd#3424 PHP errors in setup step migrate
- fd#3432 fdSaslRealm and fdSaslExop are booleans in the schema
- fd#3752 the setup doesnt find the ldap anymore when fai plugin is installed
- fd#3756 Error in the setup if the schemas are not inserted
- fd#3779 Fatal error in recovery: Non-existing type "user"
- fd#3820 Some PHP errors on password recovery page
- fd#3842 Trustmode in template is automaticaly desactivated
- fd#3843 Error when we try to lock a user
- fd#3855 Templates and hooks should be fixed
- fd#3897 Setup fails if root object is missing
- fd#3906 PHP error when I try to copy a template
- fd#3908 Create a user with template that force primary group not work
- fd#3940 the in ldap inspection phase departements should not show fusiondirectory owned branch to migration
- fd#3954 The ids in aclRole depend of the langage of FusionDirectory
- fd#3955 ACL roles should use geticon as well
- fd#3991 comment is wrong on the test for the php ldap extension
- fd#4032 verify that there is no problem in the gettext conversion for all plugins
- fd#4043 Display message about sasl but I not use it
- fd#4061 PHP error when I use a template to create a user
- fd#4063 Use a modifer for password in a template not work in SSHA
- fd#4064 PHP error when I delete a template
- fd#4073 CSV import is broken
- fd#4092 Base field should show "/" instead of being empty for root.
- fd#4093 There are HTML errors in the code
- fd#4137 Locked icon on users is broken
- fd#4141 check-deprecated should say which obsolete objectClass an entry is using
- fd#4144 Reference for ogroup not work
- fd#4148 PHP Errors when I set some informations in a template
- fd#4150 PHP error on template after I remove the userPassword

#### fusiondirectory-plugins
- fd-plugins#3242 samba group tab is broken
- fd-plugins#3255 Better error message if release is not set
- fd-plugins#3354 Error about missing FAI class when creating a debian repository
- fd-plugins#3589 Must set blocksize > 0 to cancel
- fd-plugins#3740 CSV import is broken
- fd-plugins#3745 Can’t create FAI profile
- fd-plugins#3747 We cannot set a dhcp for a system
- fd-plugins#3751 Systems plugin is broken
- fd-plugins#3795 Copy of fai objects is not working
- fd-plugins#3800 Errors about base are shown when switching from FAI to an other plugin
- fd-plugins#3802 PHP error: undefined variable: dn
- fd-plugins#3861 PHP error when we create a template
- fd-plugins#3952 We miss dhcp service icon
- fd-plugins#3958 fusiondirectory-shell don't give a prompt to create an user
- fd-plugins#4007 when installing all plugins i got error message about samba sid-rid
- fd-plugins#4009 The configured mail attribute '' is unsupported!  when all plugin at installed at once
- fd-plugins#4031 verify that there is no problem in the gettext conversion for all plugins
- fd-plugins#4037 It seem that force ssl not work for webservice
- fd-plugins#4038 Mandatory for primary dns server and mailadress not work
- fd-plugins#4039 Error when I update dns by dns service
- fd-plugins#4045 Error when I edit FAIBASE partition class
- fd-plugins#4060 Erreur plugin game lors de la création de serveur
- fd-plugins#4065 I got a PHP error when I open a package class
- fd-plugins#4072 CSV import is broken
- fd-plugins#4076 eduPersonNickName isn't save if modified and generate an error if account with eduPersonNickName is deleted
- fd-plugins#4083 the phone component should not have a nis group tab
- fd-plugins#4112 the dns plugin allow me to edit zone name and ip address in the global zone service tab
- fd-plugins#4147 check the copyright date on licence in each file
- fd-plugins#4151 we cannot save a phone because the ipHostNumber attribute is not saved

## %"FusionDirectory 1.0.8.9" - 2015-08-03

### Added

#### fusiondirectory
- fd#3930 updating the list of supported language in fusiondirectory

#### fusiondirectory-plugins
- fd-plugins#3831 we missed documentation for developers plugin
- fd-plugins#3834 we missed documentation for webservice plugin
- fd-plugins#3877 add a switch to select the good library when in wheezy or jessie for fusiondirectory-shell
- fd-plugins#3937 Add title option on the tr
- fd-plugins#3943 Add a option to not touch at the reverse zone
- fd-plugins#3946 Add copyright in the header of the ppolicy plugin files

### Changed

#### fusiondirectory
- fd#3829 Move documentation of dashboard and rolemanagement in core
- fd#3961 the DESC of each attribute and object should describe exactly the purpose of it - ported description and formatting issues to 1.0.8.x

### Fixed

#### fusiondirectory
- fd#3882 We missed jsonrpc entry in our api
- fd#3928 Doesn't work if ldap's cn=admin has password with "<" symbol
- fd#3932 Password default hashes of type CRYPT/XXX other than CRYPT/MD5 are not working
- fd#3941 fusiondirectory-setup --check-ldap should respect password encoding taken from the ldap

#### fusiondirectory-plugins
- fd-plugins#3960 fusiondirectory-shell cannot cat a user
- fd-plugins#3962 fusiondirectory-shell should handle multivaluated attributes properly

## %"FusionDirectory 1.0.8.8" - 2015-07-01

### Added

#### fusiondirectory
- fd#3705 add the to faq that if user doesnt show up check if nis.schema is installed
- fd#3824 Unix account - /bin/false
- fd#3870 fusiondirectory-setup miss the explanation of --check-deprectaed into to manpages
- fd#3878 we need an option in fusiondirectory-setup to tell us the fusiondirectory version
- fd#3879 we need an option into fusiondirectory-insert-schema to empty a schema
- fd#3887 Support apache group for Archlinux
- fd#3919 fusiondirectory-setup miss the explanation of --migrate-acls into to manpage

#### fusiondirectory-plugins
- fd-plugins#3830 we missed documentation for applications plugin
- fd-plugins#3832 we missed documentation for ejbca plugin
- fd-plugins#3833 we missed documentation for gpg plugin
- fd-plugins#3841 we missed documentation for certificates plugin

### Changed

#### fusiondirectory
- fd#3864 correct all the date and version on the man pages
- fd#3920 reorganize the command for list, write, set vars

#### fusiondirectory-plugins
- fd-plugins#3890 Vacation range should be disabled on mail methods as none implements it
- fd-plugins#3898 check if the demo plugin is current and use the current simple plugin api

### Removed

#### fusiondirectory
- fd#3917 remove the fusiondirectory-setup --migrate-repositories

#### fusiondirectory-plugins
- fd-plugins#3913 remove screenshot and demosection.php from developers packages

### Fixed

#### fusiondirectory
- fd#3884 Setup fails if root object is missing

## %"FusionDirectory 1.0.8.7" - 2015-06-05

### Added

#### fusiondirectory
- fd#3757 Let password-methods-sasl prompt for a password when a password-hook is  configured
- fd#3769 Add /sbin/nologin in shells
- fd#3776 Php error when editing a user
- fd#3808 password strength widget is broken from email redirection
- fd#3809 authorize to set the same password when using simpel password system ( not ppolicy)
- fd#3817 Viewing a group removes all members that don't have a memberUid in addition to member attribute
- fd#3826 We miss icons for plugin certificates
- fd#3837 fusiondirectory-setup doesn't handle correctly start_tls support

#### fusiondirectory-plugins
- fd-plugins#3706 Manage userCertificate attribute under User tab
- fd-plugins#3785 Add TXT values in global zone record
- fd-plugins#3815 Conversion of PEM certificate in DER (binary)
- fd-plugins#3827 We miss icons for plugin certificates

### Changed

#### fusiondirectory
- fd#3772 Password recovery cannot find a suitable password method for current hash
- fd#3810 check all copyright year in source

#### fusiondirectory-plugins
- fd-plugins#3811 check all copyright year in source
- fd-plugins#3816 Fix refresh DNS button in systems
- fd-plugins#3822 check all copyright year in source

### Removed

#### fusiondirectory
- fd#3753 Recovery password use default hash

#### fusiondirectory-plugins
- fd-plugins#3835 Documentation that we must remove

### Fixed

#### fusiondirectory
- fd#3783 Using Password recovery with the Personal plugin fails.

#### fusiondirectory-plugins
- fd-plugins#3762 when copying 4 servers at once from a departement to another
- fd-plugins#3786 We cannot add TXT value in DNS

## %"FusionDirectory 1.0.8.6" - 2015-05-27

### Added

#### fusiondirectory
- fd#3713 the group and user is different for apache on SLES 11 SP3
- fd#3746 Ppolicy should be applied in password recovery

#### fusiondirectory-plugins
- fd-plugins#3691 Export sub entries with ldapmanager
- fd-plugins#3693 the apache icon for the apache plugin doesnt appaers in dashboard

### Changed

#### fusiondirectory
- fd#3643 Ppolicy plugin should check password history

#### fusiondirectory-plugins
- fd-plugins#3720 Language setting

### Removed

#### fusiondirectory
- fd#3692 class_ldap.inc has leftover debug
- fd#3738 rfc2307bis option should be removed
- fd#3750 fusiondirectory.conf attribute configVersion is unused and should be removed

### Fixed

#### fusiondirectory
- fd#3235 When a manager changes its base the managed user attributes are changed
- fd#3604 Role icon is display for all users
- fd#3685 generateLdif() fails with bind-passwords containg '$'
- fd#3690 the setup doesnt find the ldap anymore when fai plugin is installed
- fd#3723 Missing icon
- fd#3731 A icon is missing when we set a manager
- fd#3732 When debug trace level is on, we can’t log in anymore

#### fusiondirectory-plugins
- fd-plugins#3687 Address book plugin show infos only in admin
- fd-plugins#3719 ACLs of GPG-Plugin

## %"FusionDirectory 1.0.8.5" - 2015-04-03

### Added

#### fusiondirectory
- fd#3554 New functionnality: list all members for a primary group
- fd#3586 Modifier for templates to convert german umlauts to 7-bit ASCII
- fd#3596 Dashboard plugin should go into core
- fd#3600 Dashboard should be completed
- fd#3634 the configuration of fusiondirectory entry in dashboard first tab doesnt have an icon
- fd#3667 the reset password should use the fdPrivateMail from the personnal plugin as alternative address

#### fusiondirectory-plugins
- fd-plugins#3397 Ppolicy plugin
- fd-plugins#3477 Plugin FAI
- fd-plugins#3482 Plugin FAI - Centos support
- fd-plugins#3525 the sudo plugin doesnt allow to save the sudoOrder
- fd-plugins#3527 Add the value ALL in systems and users and groups
- fd-plugins#3529 add supannRefId in supann Plugin
- fd-plugins#3534 gosaMailDeliveryMode in Group Mail Tab
- fd-plugins#3544 when adding ns record in the dns zone editor global name records it doesnt add the . at the end when saving
- fd-plugins#3609 system and newtork tab from the dashboard plugin goes to systems plugins to add the tab when needed
- fd-plugins#3610 ppolicy should add a tab to dashboard
- fd-plugins#3671 the reset password should use the fdPrivateMail from the personal plugin as alternative address

### Changed

#### fusiondirectory
- fd#3264 Hidden password for fusiondirectory-setup --check-ldap
- fd#3538 getbin.php should quote the file name
- fd#3539 User tab roles should not appear in «My account» menu
- fd#3579 We should update copyright notices to 2015
- fd#3595 The menu should be reorganized
- fd#3608 dashboard should be an exhaustive statistic plugin
- fd#3622 ppolicy should add a tab to dashboard

#### fusiondirectory-plugins
- fd-plugins#3569 after the first load of the kernel to be use in the fai tab, they should be cached for the rest of the session
- fd-plugins#3580 We should update copyright notices to 2015
- fd-plugins#3601 The menu should be reorganized
- fd-plugins#3645 Parent servers are empty
- fd-plugins#3648 debconf variables field should take utf8
- fd-plugins#3666 the dashboard doesnt show the icon associated to the fai objects and are not clickable

### Removed

#### fusiondirectory
- fd#3620 system and newtork tab from the dashboard plugin goes to systems plugins to add the tab when needed
- fd#3633 the small_warning.png icon is not present but used by the system dashboard tab

#### fusiondirectory-plugins
- fd-plugins#3561 Remove the message "Take over DNS configuration from  ..."
- fd-plugins#3607 in queue management we should remove the action create and the first 3 icons on the left
- fd-plugins#3616 Dashboard plugin should go into core
- fd-plugins#3635 the small_warning.png icon is not present but used by the system dashboard tab
- fd-plugins#3654 fai plugin empty with one repository on a server trigger an Undefined variable: prefix error
- fd-plugins#3676 remove example.ldif that is incorrect

### Fixed

#### fusiondirectory
- fd#3249 When a duplicate is found, its dn should be given
- fd#3315 Warning message during adduser process
- fd#3524 Can't create/edit users with gosaMailServer Attributes via acl system
- fd#3540 Password Recovery error for a non-existant user
- fd#3551 Invalid uri in the password recovery mail
- fd#3564 References are only checked in the base
- fd#3582 translation not complete
- fd#3590 Error message if we have an assignement with no members
- fd#3623 Cannot create a role with the same name of an DSA object
- fd#3659 error when copying a system from a departement to the root

#### fusiondirectory-plugins
- fd-plugins#3507 Error in plugin-dns documentation
- fd-plugins#3536 Plugins Quota et msg :The field 'Device' contains invalid characters!
- fd-plugins#3537 Error when I click on reference in EJBCA section
- fd-plugins#3555 editing a template inside fai provoke a crash
- fd-plugins#3557 Plugin FAI - partition LVM
- fd-plugins#3571 Informations for DHCP and DNS are lost after a copy paste
- fd-plugins#3592 the ppolicy exemple files should be changed to use a more generic dn
- fd-plugins#3621 ejbca my account link is broken
- fd-plugins#3626 Export single  entry give an error
- fd-plugins#3627 OPSI import should not have an empty filter
- fd-plugins#3628 DHCP plugin edit host error
- fd-plugins#3637 Applications plugin should not set forceSize parameter
- fd-plugins#3638 Error in cyrus template
- fd-plugins#3640 Remove a user display an error if we have ppolicy
- fd-plugins#3641 Token invalid when we use ppolicy
- fd-plugins#3642 FTPStatus attribute must default set at true
- fd-plugins#3644 Problem in regex to set release in FAI package list
- fd-plugins#3653 Uninitialized string offset: 0 in ldapmanager when importing an ldif
- fd-plugins#3663 No image for up/down in FAI profil

## %"FusionDirectory 1.0.8.4" - 2015-01-07

### Added

#### fusiondirectory
- fd#3530 the roles in the personal/roles miss an main.inc !
- fd#3531 the roles icon is missing in my account

## %"FusionDirectory 1.0.8.3" - 2014-12-15

### Added

#### fusiondirectory
- fd#3400 copyright and license is missing for include/functions_debug.inc
- fd#3419 FD should allow plugins to act as menu entries providers
- fd#3426 cleaning the desktop-management plugin
- fd#3429 fusiondirectory-insert-schema should have an option to keep ldifs files
- fd#3506 we should have in core a tab in user to be able to assing him roles
- fd#3519 Where we add users in sudo plugin their are invisible

#### fusiondirectory-plugins
- fd-plugins#3416 create a new web-application plugin
- fd-plugins#3427 in some user case we have a need to be able to add roles for users and have those users get rights for applications.
- fd-plugins#3450 create a plugin to manage certificates from ejbca
- fd-plugins#3472 create a personal plugin to store all kind of personal data
- fd-plugins#3496 EJBCA tab for the systems
- fd-plugins#3497 EJBCA miss some icons
- fd-plugins#3510 Argonaut mirror settings fields have no descriptions

### Changed

#### fusiondirectory
- fd#2613 new icon for debug-help aka diagrams has it is the same as ldapmanager
- fd#3431 correct all the date and version on the man pages
- fd#3441 Inactive tabs could have a different style
- fd#3458 making the saving of the samba attributes optional in 1.0.8.x
- fd#3468 Hook script with ldap array
- fd#3478 Message d'information concernant les propriétes que le mot de passe doit répondre
- fd#3493 Department list should be updated after a department creation
- fd#3501 the uid id naming mess should be cleaned up
- fd#3509 Should french translation use «courriel»?

#### fusiondirectory-plugins
- fd-plugins#3401 autofs/admin/autofs/main.inc have a gpl v3 licence and its wrong it should be gpl v2
- fd-plugins#3407 plugins are still using obsolete attribute cellspacing on table tags
- fd-plugins#3421 memory exhaustion when supann plugin is installed and i try to use the import csv
- fd-plugins#3423 cleaning the desktop-management plugin
- fd-plugins#3462 update the official files for the supann plugin
- fd-plugins#3502 Argonaut is sometimes called «infastructure service» and sometimes «service infrastructure»
- fd-plugins#3522 the sudo schema from the sudo-ldap package i using old openldap syntax and must be corrected and included

### Removed

#### fusiondirectory
- fd#3396 remove old gosa spec from core-fd.schema
- fd#3476 We should remove the ACL tabs from special tabs
- fd#3504 FD should stop using the word generic every where
- fd#3505 setup_feedback.tpl is a useless file
- fd#3517 remove gouvernement mode from fusiondirectory

#### fusiondirectory-plugins
- fd-plugins#3393 update-from-1.0.6-to-1.0.7 in plugin-systems
- fd-plugins#3402 remove the Authors.rst from the autofs plugin
- fd-plugins#3403 remove the glp v3 licence from the autofs plugin
- fd-plugins#3404 remove the sieve dir from the mail plugin
- fd-plugins#3408 update-from-1.0.6-to-1.0.7 in plugin-mail
- fd-plugins#3483 removing leftover of the desktop-management plugin

### Fixed

#### fusiondirectory
- fd#3368 Missing png for goServer (systems)
- fd#3369 «&amp;» should be used in icon paths
- fd#3387 When locking an account with no samba password it adds a samba attribute to it
- fd#3399 sometime when refreshing the generic page on a user a get template placeholder in login
- fd#3422 when i create a template in the user list i got a missing icon for the new user from template
- fd#3430 fusiondirectory-insert-schema man pages is wrong it misses an OPTIONS section
- fd#3434 The S placholder to remove whitespace doesnt seems to work
- fd#3436 when using apply template after having sucessfully imported user, it wrongly rewrite the users entries
- fd#3447 When debug trace level is on, we can’t log in anymore
- fd#3470 FusionDirectory in Amazon Linux AMI
- fd#3484 The user icon does not show up
- fd#3491 User ldap error are wrongly reported
- fd#3495 foreignKey on acls is handle several times
- fd#3500 Error in user management
- fd#3508 There is some french in password recovery
- fd#3516 Various problems in strings

#### fusiondirectory-plugins
- fd-plugins#3379 The quota tab must have a dependance on the unix tab
- fd-plugins#3415 application listing in the desktop plugin doesnt show the icons on the listing
- fd-plugins#3433 when importing user and no selection of an field in the csv to be the uid it crash
- fd-plugins#3442 dashboard network tab links to dns servers are wrong
- fd-plugins#3446 when creating a desktop application it doesnt save it
- fd-plugins#3454 Error in the systems plugin documentation
- fd-plugins#3459 the nis netgroup is always active
- fd-plugins#3485 The user icon does not show up
- fd-plugins#3490 GPG plugin have some errors
- fd-plugins#3511 The comment for the ssh public key is mandatory
- fd-plugins#3512 Application local seems wrong
- fd-plugins#3514 Various problems in strings
- fd-plugins#3518 remove the french inside the supann plugin
- fd-plugins#3520 the default entry in the sudo plugin is not editable anymore

## %"FusionDirectory 1.0.8.2" - 2014-09-25

### Added

#### fusiondirectory
- fd#2619 fusiondirectory-insert-schema should have a -c option to continue on error
- fd#3335 adding lighttpd support in the doc
- fd#3348 add the licence to class_IconTheme.inc
- fd#3352 lots of plugins files have no licenses at all

#### fusiondirectory-plugins
- fd-plugins#3304 fusiondirectory-shell should have login and password mandatory
- fd-plugins#3351 lots of plugins files have no licenses at all

### Changed

#### fusiondirectory
- fd#3297 Use relative path for geticon
- fd#3356 trust selection mode is broken in group and ogroups

### Removed

#### fusiondirectory-plugins
- fd-plugins#1034 remove all dsc form the sources
- fd-plugins#3372 remove the trust section from the sudo plugin

### Fixed

#### fusiondirectory
- fd#3149 Refresh the Debian install documentation
- fd#3332 the documentation on the repos to use for development  is not correct
- fd#3359 add user with templates didn't keep group affectation
- fd#3361 unable to create a template with posixGroup
- fd#3362 Bad aspect for tabs when a dialog is open
- fd#3364 we can’t cancel manager selection
- fd#3365 there is an error in userManagement template base regexp

#### fusiondirectory-plugins
- fd-plugins#3303 debugHelp have problems with tabclasses contaning dashes
- fd-plugins#3305 dhcp-fd.schema should be mentionned in dhcp plugin docs instead of dhcp.schema
- fd-plugins#3321 Cannot import csv using a template
- fd-plugins#3327 primary server in the dns service should not be numeric
- fd-plugins#3347 update the doxygen file
- fd-plugins#3350 correct the fsf address in licenses
- fd-plugins#3363 cannont add Etablissement
- fd-plugins#3373 Default action displayed an error

### Security

#### fusiondirectory
- fd#3316 possible xss in login screen

## %"FusionDirectory 1.0.8.1" - 2014-09-02

### Added

#### fusiondirectory
- fd#3060 We should try to ease migrations when schema attributes are disappearing
- fd#3134 fusiondirectory-insert-schema should allow ldap auth
- fd#3224 fusiondirectory-setup may contain help for migrating versions
- fd#3282 adding /en/ files to fusiondirectory to enable automatic update of the transifex po files

#### fusiondirectory-plugins
- fd-plugins#3204 add a manpage for fusiondirectory-shell
- fd-plugins#3283 adding /en/ files to fusiondirectory to enable automatic update of the transifex po files

### Changed

#### fusiondirectory
- fd#3213 First time role installation on a default installation
- fd#3236 fusiondirectory-setup contains duplicated code for branch creation
- fd#3239 Use some CSS3 features
- fd#3260 Apache2.4 does not allow underscores in request header variables anymore
- fd#3284 we are now allowed to use non-ascii string in gettext translation for en

#### fusiondirectory-plugins
- fd-plugins#3251 The list of objectTypes is no longer present in class_debugHelp.inc
- fd-plugins#3298 Use relative path for geticon

### Removed

#### fusiondirectory
- fd#3203 remove overlib.js leftover
- fd#3218 Remove the plugin restauration feature
- fd#3230 Snapshot type should be deprecated
- fd#3266 We must remove update-from-1.0.6-to-1.0.7 for 1.0.8.1

#### fusiondirectory-plugins
- fd-plugins#3271 Rename a server create PHP errors

### Fixed

#### fusiondirectory
- fd#3212 After the upgrade the ACL system looks broken,
- fd#3216 fusiondirectory-setup should die on ldap search error
- fd#3217 Editing the config triggers php errors
- fd#3238 Department creation is broken
- fd#3261 Improve white-space trimming of mcrypt_decrypt() results
- fd#3262 Make non-default plugin tabs browseable when the plugin is viewed in non-editable mode
- fd#3268 fusiondirectory.secrets not included via fusiondirectory-apache.conf
- fd#3274 When we migrate the acls we have an error
- fd#3287 Undefined variable lang in index.php
- fd#3290 Cannot run a postCreate Hook by using uid field as parameters

#### fusiondirectory-plugins
- fd-plugins#3189 ldap dump should escape html
- fd-plugins#3241 Supann entite regexp is wrong
- fd-plugins#3257 Allow options in sshPublicKey

### Security

#### fusiondirectory
- fd#3227 Hide userPassword on change_password postmodfy hook error message
- fd#3263 Login with expire password work if we modify the URL

## %"FusionDirectory 1.0.8" - 2014-06-30

### Added

#### fusiondirectory
- fd#1754 tools for upgrading schema
- fd#1922 Foreign key should be handled between plugins
- fd#2630 check hooks are not in the documentation
- fd#2858 merge master of simple-plugin to fusiondirectory
- fd#2898 SimpleManagement should handle better the name column
- fd#2915 in the LDAP connection setup page we should have a button in the authentifiacation section that allow us to test the conbimation of admin dn and admin password
- fd#2926 We should use freedesktop Icon Theme Specification
- fd#2979 FusionDirectory could detect old ACL format
- fd#3065 Having some default roles
- fd#3157 some icons are missing I only see the alternate text
- fd#3177 add a space modifier to remove space in template

#### fusiondirectory-plugins
- fd-plugins#343 we need a webservice for fusiondirectory
- fd-plugins#2396 create a ipmi basic plugin for FusionDirectory
- fd-plugins#2425 IPMI interfaces should have a tab for credentials
- fd-plugins#2456 create a mail method for dovecot
- fd-plugins#2496 merging the dovecot plugin
- fd-plugins#2497 merging the sogo plugin
- fd-plugins#2513 we miss an icon for sogo, repository, gpg
- fd-plugins#2581 We should have a plugin for FusionInventory
- fd-plugins#2662 the fusioninventory plugin should have a configuration plugin
- fd-plugins#2695 Dashboard should provide a link to the argonaut server
- fd-plugins#2714 Samba Domain general attributes management
- fd-plugins#2910 argonaut server service should have an option to not get packages even if a mirror is created in FusionDirectory

### Changed

#### fusiondirectory
- fd#1711 %attribute% syntax should be used
- fd#2220 Snapshot URI and user should default to the same one as FD
- fd#2313 Rethink categories and object types
- fd#2397 The code should follow code guidelines
- fd#2438 Find out why schemaCheck disable get_objectclasses
- fd#2446 TrustsAttribute should be replaced by ServersAttribute
- fd#2488 We should be able of getting a list of object from an objectType
- fd#2502 divSelectBox should be able to display column titles
- fd#2547 ldap class has unused vars and methods
- fd#2566 SnapShotDialog and SnapshotHandler have duplicated code
- fd#2596 Departments should be based on simplePlugin
- fd#2625 We should be able to create link to object edition easily
- fd#2670 Ease ACL handling
- fd#2720 Improving HTML&CSS code
- fd#2723 Default password hash is weak
- fd#2727 Password posthooks are broken
- fd#2731 Fix the 1.0.8 branch work
- fd#2839 Fix the systemSelect situation
- fd#2901 We need to clarify role situation
- fd#2928 Message dialog should be reworked
- fd#2929 LDAP dialog errors should have error style
- fd#3074 cleaning the snapshot management
- fd#3090 We can’t use /tmp for auth token anymore
- fd#3092 Add a visual effect (change header color?) when editing

#### fusiondirectory-plugins
- fd-plugins#2410 The code should follow code guidelines
- fd-plugins#2447 TrustsAttribute should be replaced by ServersAttribute
- fd-plugins#2483 Rethink categories and object types
- fd-plugins#2503 argonautLogView should not use divlist
- fd-plugins#2512 class_tabs should be cleaned
- fd-plugins#2532 applications and mimetypes should be moved to their own plugin
- fd-plugins#2548 ldap class has unused vars and methods
- fd-plugins#2550 divlist should not be used anymore
- fd-plugins#2626 We should be able to create link to object edition easily
- fd-plugins#2645 objects::ls should be used instead of config::getObjectList
- fd-plugins#2655 we need an addon for the fusioninventory plugin that list all data thats in the inventory branch
- fd-plugins#2741 systems plugin needs to be adapted to new listing feature
- fd-plugins#2744 OPSI plugin contains duplicated code
- fd-plugins#2852 rename the board plugin to dashboard
- fd-plugins#2884 Ease ACL handling
- fd-plugins#2896 More plugins should be using simpleManagement
- fd-plugins#2937 We should use freedesktop Icon Theme Specification
- fd-plugins#2949 add ipmi as a system attribute and not as a new system
- fd-plugins#2954 Samba is uselessy using plugin class
- fd-plugins#3036 Cannot redeclare class SubNodesAttribute
- fd-plugins#3088 Ajout d'adresse mail alternative vide

### Removed

#### fusiondirectory
- fd#2418 All gosaUnitTag related code should be removed
- fd#2848 Duplicated code in setup_stepFinish
- fd#3052 Debug output when not specified during setup

#### fusiondirectory-plugins
- fd-plugins#2420 All gosaUnitTag related code should be removed
- fd-plugins#2729 Samba munged attribute ACLs
- fd-plugins#2840 Fix the systemSelect situation
- fd-plugins#3099 Role plugin moved to core

### Fixed

#### fusiondirectory
- fd#2412 Reading categories should not be done in acl constructor
- fd#2432 locking conflict errors
- fd#2505 class_tabs should be cleaned
- fd#2526 User cn is not always handled correctly
- fd#2544 class_config should follow code guidelines
- fd#2549 class certificate should follow code guidelines
- fd#2551 divlist should be deleted
- fd#2552 class_filter should follow code guidelines
- fd#2553 class_msgPool should follow code guidelines
- fd#2554 functions_debug should follow guidelines
- fd#2555 class_plugin should follow code guidelines
- fd#2560 Unused local vars in setupStep_Migrate
- fd#2565 Password can't contains <
- fd#2567 password-methods should follow code guidelines
- fd#2575 Error warning header image should not break line
- fd#2609 There is an error in plugin::tpl_parse_mask
- fd#2637 Errors about attrSort redifinition
- fd#2638 listing should allow to set the baseMode outside of the xml
- fd#2673 fix config get_department documentation
- fd#2728 Locked account and samba
- fd#2732 scope one for filter is ignored
- fd#2736 Create an user and setup an acl as the same time
- fd#2738 when creating a user in a template with accent and uppercase modifier
- fd#2758 when installing FusionDirectory for the first time it put an error message about samba sid / rid right on the login page
- fd#2838 Fix filter.tpl files
- fd#2844 LDAP SIZE LIMIT should be fixed
- fd#2846 Load acl category from plManages entry
- fd#2867 The code should not fix default values
- fd#2904 setup is broken in 1.0.8 when inspecting department
- fd#2916 Group creation is broken
- fd#2917 Problem with Unix settings "Force UID/GID" and "User must change password on first login"
- fd#2918 when creating the fd-admin user in the migration phase i got an error
- fd#2938 typo error in class_DaemonEvent.inc
- fd#2977 fusiondirectory-setup --migrate-acls fails if roles branch does not exists.
- fd#3002 Improve HTML code in order to ease tests
- fd#3027 Warning the gidNumber is already in use when we edit a group
- fd#3030 Info message when we copy/paste ACL roles in the same place
- fd#3041 divSelectBox should take an html id in its constructor
- fd#3044 Cannot apply an ACL
- fd#3045 In acl assignments the base sometimes wrongly show
- fd#3066 Copy user in an other base create the user in the root base
- fd#3067 The copy of an user is not in the group of these user
- fd#3087 Renommer un groupe
- fd#3111 systemSelect crash the unix tab
- fd#3120 samba domain don't appears !
- fd#3163 Icon theme inheritance is broken

#### fusiondirectory-plugins
- fd-plugins#2511 FusionDirectory should provide valid HTML
- fd-plugins#2530 User GPG keys info are not fetched when opening the tab
- fd-plugins#2607 PHP error "Undefined property: mailgroup::$uid"
- fd-plugins#2699 jsonRPCClient should not prevent from using HTTP auth
- fd-plugins#2733 Debconf should be reworked
- fd-plugins#2837 Fix filter.tpl files
- fd-plugins#2921 the select for the systems doesn't work on 1.0.8
- fd-plugins#2922 the samba plugin is broken on 1.0.8
- fd-plugins#2930 LDAP dialog errors should have error style
- fd-plugins#2935 old board plugin still appears on fresh FD 1.0.8 install
- fd-plugins#2946 Samba tslogin is broken
- fd-plugins#3051 GUI corruption
- fd-plugins#3053 Wrong objectclass names in Dashboard schema
- fd-plugins#3058 GUI corruption
- fd-plugins#3145 when install fusiondirectory-plugins-opsi, php class couldn't be loaded
- fd-plugins#3148 Argonaut queue is crashing when trying to load FAI logs
- fd-plugins#3153 When creating an FAI branch, subbranches should be created as well
- fd-plugins#3156 some icons are missing I only see the alternate text
- fd-plugins#3164 Servers service tab causes trouble with pluginfo div
- fd-plugins#3175 unable to copy / paste opsi software list and opsi profile on fd web interface

### Security

#### fusiondirectory
- fd#2784 shell_exec should not be used in the code it is present in class_ldap.inc
- fd#3072 Password hook not block the password change if you logged in fd-admin.

## %"FusionDirectory 1.0.7.5" - 2014-09-02

### Changed

#### fusiondirectory
- fd#3223 The snapshot base is not proposed

#### fusiondirectory-plugins
- fd-plugins#3150 FAI profiles with the same name as a class are causing troubles

### Fixed

#### fusiondirectory-plugins
- fd-plugins#3159 only a part a OPSI netboot description is shown (wich isn't significant)
- fd-plugins#3174 Supann have some problems

## %"FusionDirectory 1.0.7.4" - 2014-05-19

### Added

#### fusiondirectory
- fd#3109 There is no documentation for encrypt password option
- fd#3110 fusiondirectory-setup is unable to use TLS
- fd#3137 adding brazilian into FusionDirectory

### Changed

#### fusiondirectory-plugins
- fd-plugins#3089 Clean OPSI import code to reuse it for other imports
- fd-plugins#3108 adapting the doxyfile to make it run into the jenkins

### Fixed

#### fusiondirectory
- fd#3098 the sort is not correct in the ip colum of the system list

#### fusiondirectory-plugins
- fd-plugins#3091 ssh public key issue with 1.0.7.3
- fd-plugins#3095 the sort is not correct in the ip colum of the system list

## %"FusionDirectory 1.0.7.3" - 2014-04-03

### Removed

#### fusiondirectory-plugins
- fd-plugins#2984 remove the entry for the static tftp path in argonaut fuse service

### Fixed

#### fusiondirectory
- fd#3073 Snapshot (or restore snapshot) not working

#### fusiondirectory-plugins
- fd-plugins#3015 error when trying to create the cronjob for creating debian mirror
- fd-plugins#3031 PHP error: Undefined index: use_fon_group

## %"FusionDirectory 1.0.7.2" - 2014-02-04

### Added

#### fusiondirectory
- fd#2859 Fix the template grouping situation
- fd#2897 Provide a way to show hook execution results to the user via web
- fd#2951 fusiondirectory-insert-schema need a -m option to allow to easyli insert ldif update-schema
- fd#2952 Add Swedish language
- fd#2955 add arabic language

#### fusiondirectory-plugins
- fd-plugins#2877 weblink should also be a tab for network device and printer

### Changed

#### fusiondirectory-plugins
- fd-plugins#2861 memory exhaustion when supann plugin is installed
- fd-plugins#2945 Bug with InitialProgram (MungedDial) Samba field

### Removed

#### fusiondirectory
- fd#2927 A double border is shown when there are errors at login
- fd#2942 There is some leftover of multiedit support

#### fusiondirectory-plugins
- fd-plugins#2940 PHP error: Undefined property: phoneAccount::$multi_boxes

### Fixed

#### fusiondirectory
- fd#2774 wrong size in chrome 29.0.1547.76
- fd#2847 Template bug in memberUid
- fd#2865 snapshot are broken nearly everywhere
- fd#2900 Wrong plIcon in personal/generic and personal/posix
- fd#2941 PHP error: Undefined index: passwordRecoverySalt
- fd#2947 In configuration screen hooks can't use placeholders
- fd#2962 updating the docs for install, upgrade, ldap migration

#### fusiondirectory-plugins
- fd-plugins#2868 freeradius plugin have erroneous default value for Tunnel medium type
- fd-plugins#2934 the ipmi puglin does not work
- fd-plugins#2939 ldif error in 1.0.6 to 1.0.7 upgrade LDIFs for plugin-systems

## %"FusionDirectory 1.0.7.1" - 2013-11-18

### Added

#### fusiondirectory
- fd#2830 refresh readme and install for 1.0.8
- fd#2860 make a changelog for FusionDirectory 1.0.7.1

### Changed

#### fusiondirectory
- fd#2850 Creating several templates with same login placeholder won't work

### Fixed

#### fusiondirectory
- fd#2845 user main.inc is broken
