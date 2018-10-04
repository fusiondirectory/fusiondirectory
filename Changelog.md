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


