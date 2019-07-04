## %"FusionDirectory 1.3.1" - 2019-07-04

### Added

#### fusiondirectory
- fd#5985 We don't have LDAP tab on FusionDirectory configuration

#### fusiondirectory-plugins
- fd-plugins#5911 supannRefId field is missing for "etablissement"
- fd-plugins#5938 Make %passwordClear% available to sambaAccount hooks
- fd-plugins#5939 create icons for ipmi and weblink
- fd-plugins#5940 We don't have LDAP tab on FusionDirectory configuration
- fd-plugins#5957 creating icons for the sinaps plugin

### Changed

#### fusiondirectory-plugins
- fd-plugins#5913 Avoid mass acquistion when editing group gidNumber
- fd-plugins#5948 description of a user should'nt be updated from SINAPS

### Removed

#### fusiondirectory-plugins
- fd-plugins#5901 Remove Google+ from personal social accounts tab

### Fixed

#### fusiondirectory
- fd#5942 PHP >= 7.2 triggering error when count() is called with invalid countable
- fd#5967 Systems dashboard crashes
- fd#5978 when renaming branch aka department roles are not updated correctly
- fd#5983 Write ACL on user/userRoles/groupsMembership not working when not having full user/user read right
- fd#5987 ImagickException are not catched when showing an ImageAttribute
- fd#5997 Incorrect error message when the mail cannot be sent by recovery
- fd#6006 expiration date in dashboard is 02.01.1970

#### fusiondirectory-plugins
- fd-plugins#5917 flag_ownmailbox cannot be used with partage mail method
- fd-plugins#5918 SINAPS : acquisition couldn't be sent because primary affectation isn't set
- fd-plugins#5923 cname record appear on another domain when they contain the machine cn
- fd-plugins#5935 Crash when I try to create a server with argonaut plugin and without dns plugin
- fd-plugins#5941 Error when identity is spread from SINAPS  "Cannot delete non-existing user {LDAPUUID}"
- fd-plugins#5942 Error for user-reminder about smarty
- fd-plugins#5944 add addvalues/delvalues documentation
- fd-plugins#5945 unable to add SINAPS Structure
- fd-plugins#5949 Systems dashboard crashes

## %"FusionDirectory 1.3" - 2019-03-04

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
- fd#5896 Add a generic ObjectLinkAttribute
- fd#5913 Add option to remove download button from FileTextAreaAttribute
- fd#5914 html tag should have a lang attribute
- fd#5926 From and Reply-To headers may have different value in mail_utf8

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
- fd-plugins#5775 Possibility to add/remove values from an attribute through webservice
- fd-plugins#5787 Add an option to inherit profile from group when OPSI tab is activated
- fd-plugins#5789 Posix plugin should prevent save when automatic group cannot be created
- fd-plugins#5791 Add an option to inherit localboots from group
- fd-plugins#5801 Add other source capabilities for supannTypeEntiteAffectation
- fd-plugins#5816 Audit plugin should have a way to select a range of date to show entries
- fd-plugins#5829 Create a SINAPS plugin
- fd-plugins#5830 Add domain alias table in postfix plugin
- fd-plugins#5831 Create a new plugin for adding more information related to Supann Structure
- fd-plugins#5845 Add support for FAI 5 to argonaut-fuse
- fd-plugins#5846 Add vfat as filesystem for FAI partition
- fd-plugins#5847 Add a workaround for dracut that use FQDN for hostname
- fd-plugins#5857 Add an option for multiple release mode in argonaut-fuse service configuration
- fd-plugins#5892 sinaps plugin tab should contain some text
- fd-plugins#5894 Support for SSH ed25519 keys

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
- fd#5809 Small weirdness in foreignKey handling
- fd#5838 Change /location by /Directory in contrib/apache/fusiondirectory-apache.conf
- fd#5883 Update the whole copyright to 2018
- fd#5898 Enhance HTML title handling
- fd#5902 User tabs with plSelfModify=FALSE should not show in user menu
- fd#5909 homePhone is not a PhoneNumberAttribute
- fd#5915 "ou" should be labeled "Unit" in user edition
- fd#5917 Remove useless alt attribute on img
- fd#5935 Fatal error due to CRSF security
- fd#5943 Backport LDAP size limit refactor
- fd#5944 It looks like directory does not work on /fusiondirectory in fusiondirectory-apache.conf
- fd#5946 remove no longer needed if section an the php option from fusiondirectory-apache.conf
- fd#5950 Adapt fusiondirectory-setup --encrypt-passwords output to new recommendations

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
- fd-plugins#5872 Supann labels and attribute type review
- fd-plugins#5885 supannCodeEntiteParent should allow multiple values
- fd-plugins#5893 sinaps plugin should have a main.inc
- fd-plugins#5908 Double check usages of count()

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
- fd#5888 Plugin configuration options are not available through webservice
- fd#5889 Ldap filter parsing issue
- fd#5897 Support for %askme% in BooleanAttribute is broken
- fd#5921 When having ACL rights on subbranches the tree looks funny
- fd#5938 Setup Fatal Error Uncaught UnknownClassException: setupStepConfig
- fd#5940 Error when we open dashboard

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
- fd-plugins#5841 Error in etablissement::ldap_remove
- fd-plugins#5842 Invalid call to strpos in debconfProfileGeneric
- fd-plugins#5843 In supann-ext plugin there is no check than startDate comes before endDate
- fd-plugins#5881 Audit event creation fails if there is unicode in the dn
- fd-plugins#5883 Deserialization errors are not showed during CSV import
- fd-plugins#5900 Next ID hook feature is broken
- fd-plugins#5898 Correct the missing explanation on  _('Entity assignment'), _('supannEntiteAffectation') in the supann plugin
- fd-plugins#5902 In CSV import, importing a new file should not check fields from right part
- fd-plugins#5903 Acquisition to sinaps is not sent if end contract date is empty

## %"FusionDirectory 1.2.3" - 2018-11-15

### Added

#### fusiondirectory
- fd#5890 hash_equals is not available in PHP 5.4
- fd#5901 Add the pdf with the oid of fusiondirectory in the contrib dir of the core

### Fixed

#### fusiondirectory
- fd#5797 cancel button has no effect (apply template)

#### fusiondirectory-plugins
- fd-plugins#5870 Supann should not trigger PHP errors on empty lines in nomenclature files

### Security

#### fusiondirectory
- fd#5918 Password recovery does not escape fields in error messages

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

## %"FusionDirectory 1.0.7" - 2013-11-06

### Added

#### fusiondirectory
- fd#2245 Add a test to codesniffer for the php foreach issue
- fd#2354 Config backend is not in the documentation
- fd#2406 Allow filter to have no template
- fd#2431 We should add an easy way of adding css files.
- fd#2482 We need a default value for ipmiRDN
- fd#2520 Could not load: 'UserPasswordAttribute'
- fd#2546 RTL
- fd#2564 remove mysql-dependencies in asterisk-plugin
- fd#2569 cleanup fusiondirectory.pod removing all obsoleted options
- fd#2584 Array to string conversion error when saving DNS
- fd#2618 adding a --list-ldap to fusiondirectory-setup
- fd#2631 No (obvious) way to select the pool allocation method
- fd#2658 include class_objects.inc in core
- fd#2739 add a space modifier to remove space in template
- fd#2756 Next id hook should have documentation
- fd#2815 create ldif for updating the schema in 1.0.7

#### fusiondirectory-plugins
- fd-plugins#2478 Adaptation for Dovecot plugin
- fd-plugins#2578 Update packet
- fd-plugins#2651 integrating dovecot, gpg, ipmi, repository, sogo, weblink into the stable branch
- fd-plugins#2660 we need an the icon from the fusioninventory project for our fusioninventory plugin
- fd-plugins#2668 now that we have an acl support in the ldapdump fonctionnality it should be moved out of the debug-help plugin and put onto is own plugin
- fd-plugins#2677 Add export feature in rsyslog plugin
- fd-plugins#2689 adding the fusioninventory plugin to plugins
- fd-plugins#2816 create ldif for updating the schema in 1.0.7
- fd-plugins#2825 move the developpers stuff into a developpers plugin in the plugins

### Changed

#### fusiondirectory
- fd#1097 ACL menu should show plShortName
- fd#2266 fusiondirectory-setup should fetch config in the LDAP
- fd#2390 when proofreading doc i found mention of a check hook
- fd#2408 Rework menu html code
- fd#2409 style.css should be split
- fd#2416 posixAccount should use DisplayAttribute
- fd#2435 Some clean is needed in the tpl organization.
- fd#2443 Sauvegarde acl
- fd#2457 migrate fusionddirectory-encrypt-password into fusiondirectory-setup
- fd#2489 Left menu css should be reworked
- fd#2559 Error message are weirdly displayed
- fd#2576 Some installation instruction path are wrong
- fd#2639 No .ldif since FD 1.0.6, change doc: admin_installation_from_source
- fd#2640 No more config in /etc, change doc: admin_installation_from_source
- fd#2697 moving smarty var from variables_common.inc to variables.inc

#### fusiondirectory-plugins
- fd-plugins#215 /usr/share/fusiondirectory/plugins/addons/goto/deploy-filter.tpl status
- fd-plugins#2339 "board" plugin rename to "dashboard"
- fd-plugins#2433 remove serviceDebianRepository from argonaut client tab
- fd-plugins#2434 mac should be changed back to non mandatory in system
- fd-plugins#2436 style.css should be split
- fd-plugins#2484 config directory incorrect for the mail plugin
- fd-plugins#2690 Rework postfixTransportTable
- fd-plugins#2716 Repositories should be associated to distro/section couples
- fd-plugins#2717 Repository type should be in the form

### Removed

#### fusiondirectory
- fd#2514 remove the meta code  from headers.tpl
- fd#2663 Smarty 3 vs Smarty 2
- fd#2681 class_acl changes html code depending on browser
- fd#2685 now that we have an acl support in the ldapdump fonctionnality it should be moved out of the debug-help plugin and put onto is own plugin
- fd#2687 non-JS fall-backs should be removed
- fd#2748 we should remove from fusiondirectory-setup all the old code for systems, asterisk, mail
- fd#2749 the apache config provided by the core should not include the line of config for fusiondirectory.secrets
- fd#2819 remove scriptaculous from the source
- fd#2821 remove included smarty gettext
- fd#2823 remove schema2ldif from the source
- fd#2824 move the developpers stuff into a developpers plugin in the plugins

#### fusiondirectory-plugins
- fd-plugins#2440 Some clean is needed in the tpl organization.
- fd-plugins#2561 remove mysql-dependencies in asterisk-plugin
- fd-plugins#2615 remove FakeAttribute du to moving it to simple-plugin
- fd-plugins#2688 non-JS fall-backs should be removed
- fd-plugins#2798 remove the old obsolete doc in the nergroups plugin

### Fixed

#### fusiondirectory
- fd#2340 Default value for "encodings" after installation
- fd#2384 available roles it's a wrong definition in acl
- fd#2389 Configuration windows red form not very explicit
- fd#2403 When going to objectgroups with only the core plugins i get  Undefined index: SERVERS
- fd#2429 There are wrong pattern in pod explanations
- fd#2448 postcreate syntax is wrong in some docs
- fd#2452 When /dev/random runs out of entropy, it blocks the mcrypt_create_iv function.
- fd#2463 there is an error in fusiondirectory.conf manpage
- fd#2476 Checking checkbox does not enable fields in setup
- fd#2486 Incorrect syntax for core-fd-conf schema
- fd#2494 divSelectBox should escape html code from its cells
- fd#2507 import_complete_ldif error handling is broken
- fd#2510 FusionDirectory should provide valid HTML
- fd#2515 Allow single quote in givenName
- fd#2523 correct misspelling from fusiondirectory-setup-manpage.patch and fusiondirectory-setup.patch debian patchs in core
- fd#2525 Copy paste is broken
- fd#2528 php errors in logs
- fd#2540 It is possible to add a completely blank shell to the shell list in "Configuration"
- fd#2542 French translation error
- fd#2592 PHP error: Undefined index: rtl
- fd#2593 fullwidth is broken for right sections
- fd#2617 fusiondirectory-setup --check-ldap doesnt work if password is chiffred in fusiondirectory.conf
- fd#2620 fusiondirectory-insert-schema should die when an error occurs
- fd#2635 Add the admin DN as a dummy member instead of the group DN (when using rfc2307bis)
- fd#2643 backtrace when trying to create new items with the repository plugin
- fd#2652 fusiondirectory-insert-schema fails when not cn=config
- fd#2669 forceSSL in fusiondirectory.conf doesn't work
- fd#2672 PHP error about MUST undefined
- fd#2674 php error in class_objects.inc with php PHP 5.3.3-7+squeeze16
- fd#2682 Syntax error in core-fd-conf.schema
- fd#2691 Not possible to remove login restriction (gosaLoginRestriction)
- fd#2703 the check on template for uid unicity is wrong
- fd#2710 in the ldapmanager plugin, it doesnt respect the password encryption method declared in FusionDirectory
- fd#2715 in the acl plugin is don't see the categories in the list
- fd#2718 multi-valuated settings cause problem when there is only one value
- fd#2719 management is wrongly testing ACLs for removal
- fd#2721 Password posthooks are broken
- fd#2722 objectClass and attributes are duplicated in systems-fd.schema
- fd#2726 Doxyfile contains absolute paths
- fd#2747 the fusiondirectory-setup --install-plugins doesnt install the files contained in the include dir of the plugins
- fd#2751 fusiondirectory-setup --install-plugins doesnt install the config backend of each plugin
- fd#2752 fusiondirectory-setup --install-plugins doesnt install the ldap schema of all in the plugin in the contrib dirs when usign the tarballs
- fd#2753 when installing FusionDirectory for the first time it put an error message about samba sid / rid right on the login page
- fd#2754 Next id hook is wrongly placed
- fd#2755 fusiondirectory-insert-schema should remove the ldif after an error occurs
- fd#2829 wrong dir for smarty in fusiondirectory-setup

#### fusiondirectory-plugins
- fd-plugins#1948 DHCP subsection not appearing
- fd-plugins#2375 "launching action T_Deployment.reboot" debug message
- fd-plugins#2393 case of cyrusDeleteMailbox
- fd-plugins#2404 FusionDirectory bugreport - when going to applications to create one i get PHP error: Illegal string offset 'ke
- fd-plugins#2413 Board have some useless lines
- fd-plugins#2421 Incorrect syntax for systems-fd schema
- fd-plugins#2424 Error message when i create an user
- fd-plugins#2426 removal of errors cache
- fd-plugins#2430 Migrate class_user to simple-plugin
- fd-plugins#2442 Action scheduling from deployment queue crashes
- fd-plugins#2449 modification d'un utilisateur
- fd-plugins#2474 Error deleting object from queue
- fd-plugins#2490 Board : PHP error "Invalid argument supplied for foreach()"
- fd-plugins#2493 We need to adapt a few things to last simplePlugin commit
- fd-plugins#2498 divSelectBox should escape html code from its cells
- fd-plugins#2506 ButtonAttribute has been renamed
- fd-plugins#2508 import_complete_ldif error handling is broken
- fd-plugins#2543 in the mail config backend the translation for the generic method is false
- fd-plugins#2545 the service postfix doesnt check the postfixMyHostname correctly
- fd-plugins#2574 when saving a windows workstations the uid is stored twice wrongly
- fd-plugins#2580 Deployment queue - error aborting queued jobs
- fd-plugins#2583 Array to string conversion error when saving DNS
- fd-plugins#2612 missing or wrong attribute
- fd-plugins#2636 DNS Zones unusable with bind-sdb
- fd-plugins#2675 Boolean options in sudo are always reset to FALSE
- fd-plugins#2676 log messages shouldn't be truncated in rsyslog plugin
- fd-plugins#2679 Quota plugin have some leftovers
- fd-plugins#2698 don't change stauts of opsi installed package when adding or removing packages to an opsi profile
- fd-plugins#2700 Empty translated strings should be avoided
- fd-plugins#2705 in the ldapmanager plugin, the import csv doesnt show my template
- fd-plugins#2711 the dn is wrong when saving imported user into ldap from ldapmanager
- fd-plugins#2740 when the opsi plugin is installed i always have this error even withtout an opsi server configured "Can't find IP for OPSI service"
- fd-plugins#2742 When an opsi server has not an mac address
- fd-plugins#2757 since their moving the icon for applications and mimetypes doesnt appers in the menu

## %"FusionDirectory 1.0.6" - 2013-05-10

### Added

#### fusiondirectory
- fd#2355 Severities in the FDstandard codesniffer standard should be set
- fd#2374 the ldapFollowReferrals should be in fusiondirectory.conf.pod as it is a file config option

#### fusiondirectory-plugins
- fd-plugins#2349 the dsa plugin should allow to select the password encoding method

### Changed

#### fusiondirectory
- fd#2378 Postcreate doesn't parse variables

#### fusiondirectory-plugins
- fd-plugins#2352 dsaManagement should use simpleManagement
- fd-plugins#2377 config directory incorrect for the samba plugin

### Removed

#### fusiondirectory
- fd#2356 all checkbox=$multiple_support should be remove from the smarty templates
- fd#2362 remove the update schema from 1.0.4 for 1.0.6

#### fusiondirectory-plugins
- fd-plugins#2357 all checkbox=$multiple_support should be remove from the smarty templates
- fd-plugins#2364 remove the update schema from 1.0.4 for 1.0.6
- fd-plugins#2365 remove the remaning readme into the plugins

### Fixed

#### fusiondirectory
- fd#2346 Fusiondirectory 1.0.5 + wheezy + user management
- fd#2351 case of forcePasswordDefaultHash
- fd#2361 Login uses simple quotes for translated strings in smarty template
- fd#2371 fusiondirectory-insert-schema should look for the right name

#### fusiondirectory-plugins
- fd-plugins#2336 kolab2 schema failed insertion
- fd-plugins#2350 when i save a mail group i got a not needed gosaMailDelivery mode saved

## %"FusionDirectory 1.0.5" - 2013-04-23

### Added

#### fusiondirectory
- fd#858 FD should have a PHP CodeSniffer standard to check Guidelines respect
- fd#1074 Add logoff scripts to LDAP schema
- fd#1124 Autogénération de mots de passe dans les templates
- fd#1223 New tab system
- fd#1263 Allow special characters in passwords
- fd#1336 Enhance ogroups tab handling
- fd#1353 Functions.inc check uid against 'admin'
- fd#1371 PHP error "No department mapping found for type debconfRDN"
- fd#1377 adding debconf to ogroups
- fd#1378 set_post should be added to funtions.inc and renamed
- fd#1445 Fusiondirectory configuratin should go into the LDAP
- fd#1461 Setup should write config into the LDAP
- fd#1507 fusiondirectory-insert-schema should install fusiondirectory-conf.schema
- fd#1514 adding aliasRDN default to functions.inc
- fd#1531 Trace should be given for each fatal error
- fd#1567 adding dsaRDN default to functions.inc
- fd#1594 add description field in "generic" user tabs
- fd#1746 add a new type of device : mobile device
- fd#1769 fusiondirectory-insert-schema should remove the ldif created after inserting them
- fd#1854 allow selecting a phone number from user information tab
- fd#1929 adapt the INSTALL file to include setup of the argonaut include dirs
- fd#1933 manipulate Lower/uppser case in template
- fd#1949 Add support for crypt/sha-256 and crypt/sha-512
- fd#1951 Add support for a configuration setting to force the use of a hash when setting or changing passwords
- fd#2038 FusionDirectory could have a tutorial game to discover the features
- fd#2100 add missing authors from bug reports and patches
- fd#2124 manpage of fusiondirectory.conf must be updated
- fd#2153 opsiClient should be object group compliant
- fd#2159 fusiondirectory-setup should have an option to select the ldap server to which it should connect
- fd#2261 Error when reading config should tell users to run fusiondirectory-setup --check-config

#### fusiondirectory-plugins
- fd-plugins#383 Kerberos Integration plugin
- fd-plugins#517 create a plugin to manage puppet node
- fd-plugins#531 translation of system plugin
- fd-plugins#712 the freeradius plugin should have the same entries as the dialupadmin user interface
- fd-plugins#833 an automount service would be nice
- fd-plugins#868 We need an FD plugin for debconf
- fd-plugins#956 argonaut should work on ssl mode https
- fd-plugins#1004 the dns and logging services don't have the start/stop/restart button
- fd-plugins#1056 Add vboxvideo to video driver list
- fd-plugins#1073 Add logoff scripts to user environment
- fd-plugins#1075 Add logoff script to applications
- fd-plugins#1079 Add support for logoff scripts in multi edit mode
- fd-plugins#1091 Rewrote freeradius
- fd-plugins#1141 PATCH] Add "setup and "repository" hooks.
- fd-plugins#1142 [PATCH] Add support for nouveau as xdriver (Trac #8460)
- fd-plugins#1143 [PATCH] Add commment support to various device types (#4308)
- fd-plugins#1146 PATCH] Add FAI state column.
- fd-plugins#1153 Scalix plugin must have schema files for LDAP
- fd-plugins#1167 Add a new function get_configured_packages and move logic, which gets debconf configuration for all configured packages from LDAP, to this function.
- fd-plugins#1170 Added special check when copying applications.
- fd-plugins#1294 add a new type of device : mobile device
- fd-plugins#1389 adding ldif for puppet schema
- fd-plugins#1406 When argonaut server is configured but not answering, the wait is too long
- fd-plugins#1423 icons for apace2 service
- fd-plugins#1434 password recovery documentation
- fd-plugins#1458 Fusiondirectory configuratin should go into the LDAP
- fd-plugins#1469 Argonaut-fuse should let modules read their own config
- fd-plugins#1475 New tab system
- fd-plugins#1530 there should be an icon for the sympa service in the system list
- fd-plugins#1538 Board plugin needs both dhcp and dns if systems is installed
- fd-plugins#1552 zone file and  named filed are in the same folder
- fd-plugins#1568 icon for main menu and listing is missing
- fd-plugins#1578 icon for spamassassin is wrong (only a ?)
- fd-plugins#1580 icon for the antivirus service is wrong its only a ?.
- fd-plugins#1583 Inheritance should be handled
- fd-plugins#1593 creaating a option for synchronizing UNIX expiration date to (or from)  SAMBA espiration date
- fd-plugins#1597 merging dsa plugin into core plugins
- fd-plugins#1598 merging alias plugin into core plugins
- fd-plugins#1599 merging sympa plugin into core plugins
- fd-plugins#1666 for the generic basicMail method we need a mailService
- fd-plugins#1713 when ssh plugin is installed i got a bug when click on ssh in the my account
- fd-plugins#1748 Systems plugin needs a configuration plugin
- fd-plugins#1771 the ssh plugin doenst have an icon for the my account menu
- fd-plugins#1788 move to gofon.schema to asterisk plugin
- fd-plugins#1790 move to gofax.schema to fax plugin
- fd-plugins#1794 move to goserver.schema goto.schema gosystem.schema to system plugin
- fd-plugins#1806 moving mail attributes and objectclass into the mail-fd.schema
- fd-plugins#1808 moving proxy attributes from core schema to proxy-fd.schema
- fd-plugins#1811 moving the attributes from the core schema to the systems schema
- fd-plugins#1813 moving intranet attributes from core to the intranet plugin
- fd-plugins#1815 moving the attributes from core to the webdav plugin
- fd-plugins#1819 moving the update from update-from-1.0.3 to the plugins now that the schema have been moved
- fd-plugins#1830 enhancing the ldap service
- fd-plugins#1848 allow selecting a phone number from user information tab
- fd-plugins#1850 We need an icon for mobile phones
- fd-plugins#1906 rSyslog plugin not only mysql!!
- fd-plugins#1914 Opsi client tab (management) modeled after the argonaut client tab
- fd-plugins#1917 creating an opsi service
- fd-plugins#1937 move the gosaDefaultPrinter from core schema to system.schema
- fd-plugins#1960 OPSI module should be able to handle global import
- fd-plugins#1961 when removing a windows pc, it should be removed from opsi if activated
- fd-plugins#1968 We should have a way of creating OPSI profiles
- fd-plugins#1969 jsonRPCClient.php should show JSON decode errors
- fd-plugins#1972 We should be able of setting the profile in the OPSI client tab
- fd-plugins#1993 Deployment queue should show winstation as valid targets
- fd-plugins#2018 OPSI profile should allow to select the requested action
- fd-plugins#2019 OPSI profile should allow to set product properties
- fd-plugins#2036 new icon for opsiprofiles
- fd-plugins#2037 icons of management system
- fd-plugins#2040 merge autofs into core plugins
- fd-plugins#2043 merge freeradius into fusiondirectory-plugins
- fd-plugins#2045 merge zarafa plugin into fusiondirectory-plugins
- fd-plugins#2047 merge debconf into fusiondirectory-plugins
- fd-plugins#2052 merge game into fusiondirectory-plugins
- fd-plugins#2053 merge puppet into fusiondirectory-plugins
- fd-plugins#2063 for opsi we need to manage list of products
- fd-plugins#2065 adding software on demand onto the opsi service
- fd-plugins#2077 the icon for the nagios plugin is wrong
- fd-plugins#2108 missing main.inc in personal dir of several plugin create error in web interface
- fd-plugins#2110 we need an icon for squid and pureftpd for the main menu
- fd-plugins#2117 we need icons for squid, phpscheduleit, pureftpd, zarafa for the main menu
- fd-plugins#2128 nagios-schema
- fd-plugins#2132 debconf icon
- fd-plugins#2173 make user properties dynamical
- fd-plugins#2184 netgroup plugin misses config in ldap backend
- fd-plugins#2188 moving the status_* icons to the system plugin
- fd-plugins#2228 fai branch not created when adding a repository
- fd-plugins#2256 date picker for radiusExpiration
- fd-plugins#2265 removal of option to disable sambaLMPassword
- fd-plugins#2269 new icons for sudo-plugin
- fd-plugins#2290 OPSI should have icons for OPSI profile, software list and ondemand list
- fd-plugins#2294 no snapshot fonctionality on some plugins
- fd-plugins#2316 board additional parameters
- fd-plugins#2318 add serviceOpsi = opsiconfd in argonaut client

### Changed

#### fusiondirectory
- fd#216 Missing translation on the index.php file
- fd#256 Icone in properties column when listing all itme isn't dynamique
- fd#981 Improve the FusionDirectory API
- fd#1026 those services should be renamed to be more in line with the others
- fd#1057 Optimize LDAP searchfilter. The LDAP-Performance-Warning after Gosa-Login doesn't appear anymore.
- fd#1085 Plugins configuration files not stored in /etc
- fd#1090 plugin should treat objectClass in a case insensitive way.
- fd#1126 id-generator différent suivant le template
- fd#1233 Small string change in core
- fd#1255 next button in welcome wizard setup
- fd#1312 The css style for tables should be re-organized
- fd#1315 Reorganize menu
- fd#1352 Some tests::is_url should be checked
- fd#1355 style.css should be adapted to have some css about simplePlugin
- fd#1443 servtabs and worktabs should be renamed
- fd#1471 workstationGeneric should use simplePlugin
- fd#1501 userMainInc is deprecated
- fd#1504 fusiondirectory-insert-schema should be able of inserting .schema files
- fd#1505 setup doesnt store the config in the ldap server
- fd#1506 copy/paste should be put to yes into the setup
- fd#1522 you are edting a databse entry removal
- fd#1528 the ldap sizelimit should be in to ldap config backend and used by the software
- fd#1537 configuration screen
- fd#1556 Connectivity should use the brand new multiPlugin class
- fd#1566 separator and plugbottom class should be enhanced
- fd#1626 posixAccount shouild use simplePlugin
- fd#1664 rename the imap service to cyrusService
- fd#1708 Admin handling in fusiondirectory-setup seems wrong
- fd#1712 look for all unecessary tpl in the plugins
- fd#1727 the management of the windows workstations in the samba plugin should be migrated to simple plugin
- fd#1734 serverService should not use a plugin to display services list
- fd#1743 User image code does not do what it should
- fd#1803 rename the gosa-samba3.schema to gosa.schema
- fd#1817 rename update from-1.0.3 to update-from-1.0.3-to-1.0.4
- fd#1836 rename fusiondirectory-conf.schema to core-fd-conf.schema
- fd#1837 rename recovery.schema to recovery-fd.schema
- fd#1838 change the name of fusiondirectory-conf.schema and recovery.schema inside fusiondirectory-insert-schema
- fd#1899 divSelectBox should use the new style organisation
- fd#1907 Using preg_match in CompositeAttribute
- fd#1910 move the supann file back to /etc/fusiondirectory/supann
- fd#1921 check_services in setupStep_Migrate should check new LDAP service
- fd#1935 correct the  contributed slapd.conf to use the correct schema
- fd#1956 locale .po file are no longer in in locale/ca/LC_MESSAGES/messages.po
- fd#2061 uid generator and uid in template
- fd#2076 make user properties dynamical
- fd#2097 README.cnconfig should be updated
- fd#2127 config should store more information about objectTypes
- fd#2158 Duplicate objectType error message could be more detailed
- fd#2163 Main menu is ugly
- fd#2169 Snaphots should be tested
- fd#2177 clean up and create icon in big and small and mini format
- fd#2190 standardizing the icon and mini icons naming
- fd#2193 standardizing the icon and mini icons naming
- fd#2216 removal of option to disable sambaLMPassword
- fd#2309 moving localization files from messages.po to fusiondirectory.po
- fd#2333 fd-insert-schema should use schema2ldif

#### fusiondirectory-plugins
- fd-plugins#717 Variable name consistency should be check and fixed
- fd-plugins#927 those services should be renamed to be more in line with the others
- fd-plugins#991 Some plugins should use simplePlugin
- fd-plugins#1006 Wrong name for 0-freeradius.ldif
- fd-plugins#1008 freeradius plugin codereview
- fd-plugins#1060 Allow dashes in share names.
- fd-plugins#1070 Only allow [A-Za-z0-9_] in class names
- fd-plugins#1076 serverService should be reworked
- fd-plugins#1078 FusionDirectory should allow more than one fax, kerberos, cups server
- fd-plugins#1094 Rewrote nagios to extends simplePlugin
- fd-plugins#1098 Rewrote pureftpd plugin to extends simplePlugin
- fd-plugins#1099 Rewrote phpscheduleit plugin to extends simplePlugin
- fd-plugins#1101 Rewrote pptp plugin to extend simplePlugin
- fd-plugins#1102 Rewrote squid plugin to extend simplePlugin
- fd-plugins#1103 Rewrote openstack-compute plugin to extend simplePlugin
- fd-plugins#1107 migrate the plugin to the simple plugin framework
- fd-plugins#1119 Rewrote rsyslog plugin to extend simplePlugin
- fd-plugins#1123 Rewrote webdav plugin to extend simplePlugin
- fd-plugins#1127 Rewrote Intrenet plugin, in connectivity to extend simplePlugin
- fd-plugins#1135 argonautClient should use simplePlugin
- fd-plugins#1136 Rewrite the rolemanagement plugin with simplePlugin
- fd-plugins#1139 [PATCH] Take over profile options from templates
- fd-plugins#1140 [PATCH] Take over kiosk profile options from template
- fd-plugins#1150 Asterisk plugin should use simplePlugin
- fd-plugins#1151 All plugins using section templates must be updated
- fd-plugins#1154 Migrate scalix plugin to simplePlugin framework
- fd-plugins#1168 Use FAI::get_all_objects_for_given_base to get the list of FAI classes and limit the scope to the current FAI release.
- fd-plugins#1169 Move check for used workstations so that the check is executed after adding a workstation instead of complaining on save.
- fd-plugins#1177 Rewrote terminal service
- fd-plugins#1182 Rewrote sudo plugin to extend simplePlugin
- fd-plugins#1184 Rewrote virus service
- fd-plugins#1186 Rewrote imap service
- fd-plugins#1187 Rewrote ntp service
- fd-plugins#1188 Rewrote fax service
- fd-plugins#1201 Rewrote apache plugin to extend simplePlugin
- fd-plugins#1210 Rewrite kolab service with simpleService
- fd-plugins#1212 Rewrote mail plugin (userTab)
- fd-plugins#1213 Rewrote mail service
- fd-plugins#1218 Rewrote spam service, in mail admin
- fd-plugins#1231 Rewrote kolab plugin, in connectivity
- fd-plugins#1253 Rewrite argonaut service
- fd-plugins#1262 systems should not be saved with fai class by default
- fd-plugins#1322 boot kernel should not depend on fai
- fd-plugins#1338 workstationGeneric should use simplePlugin
- fd-plugins#1341 termDNS should be renamed
- fd-plugins#1379 htmlencode should be used instead of inexisting set_post
- fd-plugins#1381 (re)start and stop buttons should not depend on some weird LDAP field
- fd-plugins#1384 Apache service should only allow one ScriptAlias
- fd-plugins#1387 making puppet plugin conform to plugin directory specification
- fd-plugins#1427 (re)start and stop buttons should not depend on some weird LDAP field
- fd-plugins#1432 servtabs and worktabs should be renamed
- fd-plugins#1510 ip/mac should be mandatory
- fd-plugins#1529 service ldap should be converted to simple plugin
- fd-plugins#1555 Connectivity should use the brand new multiPlugin class
- fd-plugins#1590 Terminal tabs should be using simple plugin
- fd-plugins#1596 Terminal tabs should be using simple plugin
- fd-plugins#1612 fdquota.ldif le schema devrait etre renome
- fd-plugins#1615 BootKernelAttribute should not be in faiStartup
- fd-plugins#1621 we need a mailmethod that only store data into the ldap without connection to a server
- fd-plugins#1623 The css style for tables should be re-organized
- fd-plugins#1633 system deployement should be preplaced by  system queue or something more explicit
- fd-plugins#1634 in the argonaut server service timeout the field timeout should be at 10 by default
- fd-plugins#1667 the cyrus mail method and service should be moved in his own plugin
- fd-plugins#1670 serviceMail should be renomed servicePostfix as its about postfix stuff only
- fd-plugins#1683 the icon used in user list for samba come from  the system plugin
- fd-plugins#1684 the icon used in object group list for samba user come from  the system plugin
- fd-plugins#1686 SSH should be using simplePlugin
- fd-plugins#1689 Undefined mailMethod should be renamed
- fd-plugins#1701 Migrate repository service to simplePlugin
- fd-plugins#1702 Clean up the DaemonEvent mess
- fd-plugins#1715 sambaAccount shoud be using simplePlugin
- fd-plugins#1725 the management of the windows workstations in the samba plugin should be migrated to simple plugin
- fd-plugins#1728 fai_repository.tpl must be undeleted
- fd-plugins#1729 Share service should be using simpleService
- fd-plugins#1733 serverService should not use a plugin to display services list
- fd-plugins#1735 Applications plugin should be using simplePlugin
- fd-plugins#1742 we should put all schema in all plugin into contrib/openldap to be standard
- fd-plugins#1766 ip/mac not mandatory in the case of mobile phone
- fd-plugins#1772 Plugins configuration files not stored in /etc
- fd-plugins#1792 move to goto-mime.schema to system plugin
- fd-plugins#1801 move all sql files inside contrib/sql
- fd-plugins#1802 rename gofon.sql to asteriskcdrdb.sql
- fd-plugins#1820 rename update from-1.0.3 to update-from-1.0.3-to-1.0.4
- fd-plugins#1826 Application should use FileTextAreaAttribute
- fd-plugins#1829 environment must use simplePlugin
- fd-plugins#1844 moving the sieve*.txt scripts into the cyrus plugin
- fd-plugins#1858 SUPANN structures should use simplePlugin
- fd-plugins#1864 rename gofon.schema to asterisk-fd.schema
- fd-plugins#1866 rename gofax.schema to fax-fd.schema
- fd-plugins#1868 rename argonaut.schema to argonaut-fd.schema
- fd-plugins#1870 rename dhcp.schema to dhcp-fd.schema
- fd-plugins#1872 rename dsa-fd.schema to dsa-fd-conf.schema
- fd-plugins#1874 splitting the mail-f.schema into mail-fd.schema and mail-fd-conf.schema
- fd-plugins#1878 rename the fd-samba-conf.schema to samba-fd-conf.schema
- fd-plugins#1880 rename the supann-fd.schema to supann-fd-conf.schema
- fd-plugins#1881 splitting the alias-fd.schema into alias-fd.schema and alias-fd-conf.schema
- fd-plugins#1889 as we changed the way some mail flags where used, we have to check all mail methods for this
- fd-plugins#1892 rename gosaLogDB into goLogDB
- fd-plugins#1894 quota plugin should use simplePlugin
- fd-plugins#1905 SupAnn should use simplePlugin
- fd-plugins#1908 Using preg_match in CompositeAttribute
- fd-plugins#1923 faiStartup needs its mask redone
- fd-plugins#1938 acl should be renomed cyrusAcl and should be moved to the cyrus-fd.schema
- fd-plugins#1965 gotomasses needs an update
- fd-plugins#1999 OPSI server service url could have a default value
- fd-plugins#2003 modify opsi-import
- fd-plugins#2013 the action on all the system should be install in place of reinstall in the web interface
- fd-plugins#2015 when a system is locked there sould be no order send to him
- fd-plugins#2027 Some plugin could use simpleTabs
- fd-plugins#2031 opsiClient should be object group compliant
- fd-plugins#2033 the creation of hooks, template etc ... doesnt work anymore in fai plugin
- fd-plugins#2057 cleanup the freeradius plugin
- fd-plugins#2058 cleanup the zarafa plugin
- fd-plugins#2071 reorganize the admin and addons menu
- fd-plugins#2075 cleaning of connectivity
- fd-plugins#2129 ldapmanager tabs
- fd-plugins#2143 Cyrus service config dialog confusing
- fd-plugins#2167 sambaPwdMustChange is obsolete
- fd-plugins#2168 rewrote the main inc when plugin have been migrated to simple plugin
- fd-plugins#2170 sambaPwdCanChange is obsolete
- fd-plugins#2175 Fatal error: Call to undefined method netgroupAccount::mainInc()
- fd-plugins#2185 the rest of the netgroup plugin should be migrated to simple plugin
- fd-plugins#2187 cleaning up the mail goup tab
- fd-plugins#2196 icons for ssh plugin
- fd-plugins#2221 Snaphots should be tested
- fd-plugins#2234 reorganize the user tab
- fd-plugins#2257 the autofs-fd.schema should be renamed to autofs-fd-conf.schema
- fd-plugins#2267 No icon in ldapmanager export/import
- fd-plugins#2268 no sudoers ou created after sudo-plugin installation
- fd-plugins#2277 rename the service starting with go on the server name
- fd-plugins#2278 rename all the goto leftover to argonaut
- fd-plugins#2279 Invalid syntax - objectClass: lconfContact
- fd-plugins#2285 OPSI Admin should be renamed
- fd-plugins#2286 error when removing nis netgroup entry
- fd-plugins#2299 in the sudo plugin the add a system button show a list of user
- fd-plugins#2310 moving localization files from messages.po to fusiondirectory.po
- fd-plugins#2311 when trying to create a profile into fai fusiondirectory crashe
- fd-plugins#2320 moving cyrus-imap service name from service imap to service cyrus in argonaut client
- fd-plugins#2325 Systems action menu is broken

### Removed

#### fusiondirectory
- fd#965 remove the LC_messages from the locale directory to ease translation and packaging
- fd#1015 update_entries function should be remove from supportDaemon
- fd#1222 remov the access to the perl code for samba hash generation
- fd#1477 suppress incoming and arpnew device
- fd#1521 we should remove the code for the certificates management as it old and wrong
- fd#1554 multiple edition should be removed
- fd#1618 Main menu message should be deleted
- fd#1688 looking for old samba code and classes inside fusiondirectory core
- fd#1692 serviceMail should be renomed servicePostfix as its about postfix stuff only
- fd#1704 Migrate repository service to simplePlugin
- fd#1753 when creating a user, template aren't taken correctly
- fd#1773 remove all the xxxStatus from the goserver.schema
- fd#1774 there is not glpi plugin anymore but attributes are still in the goserver.schema
- fd#1780 remove old kerberos attribute from the schema
- fd#1786 remove the unused gosaLogServer from the goserver.schema
- fd#1787 move to gofon.schema to asterisk plugin
- fd#1789 move to gofax.schema to fax plugin
- fd#1791 move to goto-mime.schema to system plugin
- fd#1793 move to goserver.schema goto.schema gosystem.schema to system plugin
- fd#1805 moving mail attributes and objectclass into the mail-fd.schema
- fd#1807 moving proxy attributes from core schema to proxy-fd.schema
- fd#1809 remove obsolete attribute from gosa-samba3.schema
- fd#1810 moving the attributes from the core schema to the systems schema
- fd#1812 moving intranet attributes from core to the intranet plugin
- fd#1814 moving the attributes from core to the webdav plugin
- fd#1816 remove from fusiondirectory-schema all the schema that are now transfered into there plugins
- fd#1818 moving the update from update-from-1.0.3 to the plugins now that the schema have been moved
- fd#1891 removing the the objectclasses now in each plugin from the schema check in functions.inc
- fd#1909 removing quota and share from the fusiondirectory.conf
- fd#1932 objectGroup don"t save when i click on inherited
- fd#1936 move the gosaDefaultPrinter from core schema to system.schema
- fd#1980 removing the message.po file form the locale dir
- fd#1991 remove non needed anymore directories creation from fusiondirectory-setup
- fd#2094 remove fusiondirectory.mkntpassword from fusiondirectory manpages
- fd#2095 FAQ should be rewritten or removed
- fd#2099 remove GOsa_MultiSelectHeader.ihtml from fusiondirectory
- fd#2125 sudo should not be in the fusiondirectory.conf
- fd#2189 moving the status_* icons to the system plugin

#### fusiondirectory-plugins
- fd-plugins#966 remove the LC_messages from the locale directory to ease translation and packaging
- fd-plugins#1203 Netatalk plugin seems broken
- fd-plugins#1370 the deconf plugin should appears like only one entry in the admin menu
- fd-plugins#1476 suppress incoming and arpnew device
- fd-plugins#1497 Error in service dialog when none chosen
- fd-plugins#1532 System info tab should be deleted
- fd-plugins#1563 multiple edition should be removed
- fd-plugins#1579 in the imap service we got checkbox for starting and stop imap and pop services
- fd-plugins#1611 Samba hash hook should be removed from the config backend plugin of samba
- fd-plugins#1627 remove all the ldif from the plugins
- fd-plugins#1639 UserGroupsAttribute should go into simple-plugin
- fd-plugins#1693 when using the fai summary tab PHP error "Undefined index: FAIclass"
- fd-plugins#1697 look for all unecessary tpl in the plugins
- fd-plugins#1699 plDepends should be used to handle tab dependency
- fd-plugins#1740 remove the ldif files from the project
- fd-plugins#1741 the postfix class and attributes should be moved from the goserver schema to a postfix schema
- fd-plugins#1775 remove all the xxxStatus from the goserver.schema
- fd-plugins#1776 remove the kiosk service from the system plugin
- fd-plugins#1782 remove the profiles and resolution changing from user/environment
- fd-plugins#1852 remove the fonction to change password for systems
- fd-plugins#1853 remove the vacation dir from the mail plugin
- fd-plugins#1893 remove gotoLastUser from schema and systems filter
- fd-plugins#1924 systems-fd.schema should not depend of asterisk-fd.schema
- fd-plugins#1947 duplicate user tabs
- fd-plugins#1981 removing the message.po file form the locale dir
- fd-plugins#2012 the code and entry on the menu have to be removed for rescan
- fd-plugins#2021 removing goComment from server service, its of no use there
- fd-plugins#2202 removing the scalix plugin
- fd-plugins#2260 remove readme dans free radius
- fd-plugins#2289 simpleManagement does not require remove.tpl anymore
- fd-plugins#2298 remove the release filter from applicatons and mime type filter
- fd-plugins#2302 remove the broken netatalk plugin
- fd-plugins#2307 remove the phpscheduleit plugin
- fd-plugins#2315 hotplug should be removed from systems
- fd-plugins#2326 remove the on demand fonctionnality from opsi profile

### Fixed

#### fusiondirectory
- fd#509 user with OpenldapPerson class
- fd#835 FusionDirectory should allow more than one fax, kerberos, cups server
- fd#1027 Fixed problems with unsaved user pictures, due to caches file sizes. Thanks to 'arodier' (bug 1180 gosa)
- fd#1028 gosaLoginRestriction ignores ACL
- fd#1030 Do not create a group for templates. (from gosa Changeset 21143)
- fd#1046 Move reset handling before object creation, otherwise no     new object is created, when _GET['reset'] is set to 1.
- fd#1050 Fix check for used DNs
- fd#1065 Fix a bug in filterLDAP::get_list which gets triggered, if the class is used with multiple storage points and the search shall return objects in the search base.
- fd#1066 Fix #2186: Fix tooltip for menu categories
- fd#1067 Fix #6591: Strip CR from logon scripts
- fd#1068 Show description on user's hotplug devices
- fd#1087 non-functional menu "Action" under IE9
- fd#1089 Bad encoding format for supann schema and ldif files
- fd#1110 Fix wrong sizelimit warning (#8319) in groups
- fd#1111 [PATCH] #8280: Decode HTML entities in CSV exporter
- fd#1113 [PATCH] #8290: Generic fix for DOS linebreaks
- fd#1206 lines 153 and 156 of ogroupManagement seems wrong
- fd#1366 Missing breakline in setup
- fd#1447 some files miss a copyright header
- fd#1457 Some things are wrong in the man of fusiondirectory.conf
- fd#1459 timezone should be properly set by main.php
- fd#1470 sender error in password recovery
- fd#1472 Editing the config appears to break the tab systems
- fd#1478 rsyslog Smarty error
- fd#1479 plSection should only be set in plInfo of plugin that must appear in the menu
- fd#1492 Seg fault error after upgrade to debian package 1.0.4-1
- fd#1502 config debugLevel is not applying as it should
- fd#1511 when going to groups i got this error
- fd#1518 in the step inspect ldap i get those errors
- fd#1519 Copy/Paste should be fixed
- fd#1536 fatal error when trying to instyall a new
- fd#1542 copy paste for user having generic/unix/samba/mail attributes
- fd#1545 error in ldif to due bogus convertion
- fd#1550 ldapmanager error
- fd#1551 entry deletion is broken in management
- fd#1553 schema2ldif regexp is wrong
- fd#1565 Error in fusiondirectory-conf.schema
- fd#1572 hook not useable anymore
- fd#1577 favicon in fusiondirectory/html is wrong
- fd#1585 template from Gosda2.6 aren't migrate/ shown correctly ion FD 1.0.5-dev
- fd#1602 Login must be mandatory for user templates too
- fd#1609 Installer: interface for chosing LDAP Admin account has bug
- fd#1636 Same UID in differents LDAP branches - Issues with FD
- fd#1647 the fai summary tab say plugin is not activated
- fd#1648 FusionDirectory bugreport - copy paste of a user with generic / posix / samba
- fd#1649 when i copy a group it doesnt generate a new guid
- fd#1656 when the debconf plugin is installed but not activate in a system i get Error: Invalid syntax - objectClass: debconfStartup
- fd#1661 Sections should not be used for anything but display
- fd#1671 with the latest change for the mailMethod i can no longer log into Fusiondirectory
- fd#1672 when copying a user it doesnt generate a new uidnumber and guidnumber
- fd#1678 when creating a new group : PHP error: Undefined index: SAMBA
- fd#1685 plDepends should be used to handle tab dependency
- fd#1687 the warning that tell if the session is encrypted or not doesnt show up on the login screen
- fd#1705 remove gosa leftover in the fusiondirectory.conf
- fd#1714 duplicate arrtribute in the config ldap backend for mail
- fd#1717 Recovery password not working in tls
- fd#1723 CodeSniffer error : 'return;' is not allowed
- fd#1724 CodeSniffer error : do while is triggerring errors
- fd#1736 error when going to the generic in my account PHP error "Undefined property: user::$cert_dialog"
- fd#1744 User password is broken
- fd#1747 serverService should not use a plugin to display services list
- fd#1752 error when creating a user
- fd#1756 posixAccount does not return error messages
- fd#1770 status in unix tab is always expired
- fd#1857 check all the plugin for ("") in the code
- fd#1859 error when saving a template
- fd#1863 schema for storing OS of mobile phone autorize only number
- fd#1901 User image is altered by editing and saving
- fd#1930 We should be able of hiding debug
- fd#1950 The system does not use the passwordDefaultHash for new users
- fd#1953 locale is broken for fusiondirectory-core
- fd#1994 Debug output should be displayed by default
- fd#2039 Uid should be filled in user template if it's not
- fd#2042 template editing trigger a PHP error
- fd#2062 the template don't escape the html inside itself
- fd#2066 uidNumber not update when create user by using a template
- fd#2096 INSTALL file should be updated from the documentation source into the fusiondirectory package
- fd#2098 the fusiondirectory-encrypt-password file contains wrong example for the apache config
- fd#2101 update the copyright in all plugins
- fd#2103 The order of acl constructor attributes is wrong in some places
- fd#2131 Check foreach code
- fd#2138 PHP Fatal error:  Call to undefined function html_trace() in /var/www/html/fusiondirectory/include/php_setup.inc on line 138
- fd#2142 gosaMailAccount is used in core but is not in core.schema
- fd#2160 unknown attributes since new core installation
- fd#2161 User class should check roleGeneric is available
- fd#2164 We should be able of clicking anywhere on lleftmenu items
- fd#2165 when copy/pasting a disk in fai i got this error
- fd#2172 size of windows for managing properties of opsi products
- fd#2181 bug in users listing related to php_setup.inc
- fd#2192 rfc2307bis entry error in configuration
- fd#2219 reorganize the admin and addons menu
- fd#2225 cannot insert mail-fd.schema attribut is missing
- fd#2242 localization don't work
- fd#2287 user miniicon  don't appear in sudo-plugin
- fd#2293 error when removing nis netgroup entry
- fd#2305 in object groups the icons are wrong for users and groups in the list
- fd#2312 get_ou should always return a 1 comma ended string

#### fusiondirectory-plugins
- fd-plugins#617 bug in winstations
- fd-plugins#680 the session expire on is not correctly saved
- fd-plugins#681 the getvlan.pl script need to be cleaned
- fd-plugins#682 the script getvlan.pl need to read a config file and not have var to be changed inside
- fd-plugins#684 the save grouphunt doesnt exist :)
- fd-plugins#852 Code does not follow FD guidelines
- fd-plugins#1007 Missing $cn var in class_freeradiusGroup.inc
- fd-plugins#1013 Code does not follow FD guidelines
- fd-plugins#1029 sieve login does not work if plain login is only available under a layer (gosa bug 1175)
- fd-plugins#1035 Fixed problems with mail folder retrieval while dot separated account ids were used (gosa rev 21153)
- fd-plugins#1042 Web interface silently fails with passwords > 40 characters in length (gosa bug 1168)
- fd-plugins#1043 [PATCH] Forward port revision 13557 - taken from limux-gosa
- fd-plugins#1044 error when adding freeradius group
- fd-plugins#1045 Move reset handling before object creation, otherwise no     new object is created, when _GET['reset'] is set to 1.
- fd-plugins#1047 Use faiPackageList instead of faiPackage because thats actually     the right objectclass for package lists.
- fd-plugins#1048 Make repository class only conflict with itself (so     that a repository service cannot be added twice) instead     of conflicting with goKioskService,
- fd-plugins#1049 Change button label from "Import" to "Import script"
- fd-plugins#1051 Check for invalid names first before trimming name, catching trailing spaces at the end or beginning
- fd-plugins#1052 Make error message for the case when trying to create a     share with special characters more informative.
- fd-plugins#1053 Disallow '\' and '"' in object group names as they lead to LDAP     errors
- fd-plugins#1054 Use 'Device name' as string in error messages for wrong device name attributes
- fd-plugins#1055 Add type attribute to CopyPasteVars
- fd-plugins#1058 Convert vendorId and productId to lower case. Udev-rules only match if vendorId and productId are lower case.
- fd-plugins#1059 Make CopyPasteVars complete, Force adding the new object to eventually existing printer objects on Copy/Paste
- fd-plugins#1061 Run code to update printer objects not only when a printer is added or removed, but also in case the group name changes
- fd-plugins#1062 When moving or renaming devices, check whether they are in use by a user, group or object group
- fd-plugins#1063 When copying workstations / terminals additional copy some missing attributes (FAIstate, gotoSysStatus, etc.)
- fd-plugins#1064 Fix problems with removing services after saving service
- fd-plugins#1071 Trim whitespace at the end of template filenames
- fd-plugins#1072 Trim printer URI when saving object, to avoid problems, when copy-pasting an URL into the URL field
- fd-plugins#1077 Fix DOS line breaks in scripts
- fd-plugins#1080 In printerPPDDialog::SelectPPD(), check whether PPD file source and destination are identical and abort copying files over in this case.
- fd-plugins#1088 tabs_server constructor seems wrong
- fd-plugins#1114 [PATCH] #8609: Fix “Using $this when not in object context”
- fd-plugins#1137 [PATCH] Fix #7143: In printerPPDDialog::SelectPPD(), check whether PPD file source and  destination are identical and abort copying files over in this case.
- fd-plugins#1138 [PATCH] Reduce code duplication in class_environment - [PATCH] Extend features of user templates (Trac: #6032)
- fd-plugins#1145 [PATCH] #8449: Fix ACL class for hotplug device selection
- fd-plugins#1147 [PATCH] Forward port revision 13784
- fd-plugins#1152 Missing uppercases on menu Labels
- fd-plugins#1166 Disallow usage of an existing FAI profile as name for a FAI class created by copy and paste
- fd-plugins#1171 Fix wrong mountpoint display and wrong partition type, by setting mountpoint explicit to - (as required for FAI)
- fd-plugins#1172 Do not check if gosaApplicationFlags changed as it appears to be not neccessary and causes bugs
- fd-plugins#1272 check that when changing the name of a workstation, winstations, pc the name is changed inside the objects group
- fd-plugins#1383 We should test if the class_supportDaemon.inc is available
- fd-plugins#1407 Edit anyway does not work on Apache vhosts
- fd-plugins#1408 Board plugin counts are wrong
- fd-plugins#1419 supportDaemon should follow code GUIDELINES
- fd-plugins#1425 wrong copyright
- fd-plugins#1429 Argonaut services should be checked
- fd-plugins#1456 Forgot a print_r in service apache
- fd-plugins#1480 plSection should only be set in plInfo of plugin that must appear in the menu
- fd-plugins#1483 syntax error in class_environment
- fd-plugins#1484 typo in mailAccount
- fd-plugins#1489 board plugin error in readme file
- fd-plugins#1493 DHCP dialog issue
- fd-plugins#1496 Board tabs are broken
- fd-plugins#1508 error message when clicking on a printer : Fatal error: Call to a member function getBaseObject() on a non-object
- fd-plugins#1509 when opening a workstation i got two generic tab, the second is the printer one
- fd-plugins#1515 adding a config for alias to the configutation system
- fd-plugins#1516 printGeneric have an old plSection
- fd-plugins#1520 the alias plugin should support departement
- fd-plugins#1523 Copy/Paste should be fixed
- fd-plugins#1524 we need pictures in the second collum to say what kind of list this is
- fd-plugins#1526 fix copyright issues
- fd-plugins#1527 fix copyright issues
- fd-plugins#1533 Missing uppercase for the conference item
- fd-plugins#1535 field of MAC and IP are disabled
- fd-plugins#1539 fatal error when trying to instyall a new
- fd-plugins#1540 when modifying an entry it doesnt save it in the right branch
- fd-plugins#1541 copy paste should work on aliases too
- fd-plugins#1546 error in ldif to due bogus convertion
- fd-plugins#1557 argonaut error in restart from interface
- fd-plugins#1559 edition is not possible in the nova plugin
- fd-plugins#1569 renaming /deleting a system -> incorrect ldap changes
- fd-plugins#1581 critical error in supann plugin
- fd-plugins#1587 gosaMailDeliveryMode aren't correct flagged
- fd-plugins#1588 in Mail tabs on a user : if soem fieald are missing, they are filled by default values
- fd-plugins#1589 in Mail tabs on a user : some field ared filled by default value but saved anywhere
- fd-plugins#1600 alias, sympa, dsa contains a false string for the deletion of objects
- fd-plugins#1610 even with the samba plugin installed it doesnt take the default for samba-sid / samba-rid
- fd-plugins#1614 the pptp plugin doesnt allow to save the user when its installed in connectivity
- fd-plugins#1620 mailAccount is messing with save if we don't visit the tab
- fd-plugins#1622 samba-sid and samba-ri in samba config backend should have default values
- fd-plugins#1629 the fai summary tab say plugin is not activated
- fd-plugins#1632 image path bug in smarty template
- fd-plugins#1635 inclusion of a patch from the limux code use in_array_strict that was added to gosa 2.7 it should be replaced by in_array or equivalent
- fd-plugins#1642 __set and __get should be used to access attributes directly
- fd-plugins#1644 unable to reload DNS zone from fusionDirectory interface
- fd-plugins#1646 puppetNode have wrong setting in plInfo/plSection
- fd-plugins#1650 Undefined property: sambaAccount::$multiple_support_active
- fd-plugins#1651 when creating a user with a samba account : Cannot convert primary group to samba group: group cannot be identified!
- fd-plugins#1653 FusionDirectory bugreport - error when generation group for samba from unix
- fd-plugins#1655 when the debconf plugin is installed but not activate in a system i get Error: Invalid syntax - objectClass: debconfStartup
- fd-plugins#1657 Quota plugin template have a space at the wrong place
- fd-plugins#1660 Fatal error: Call to a member function cd() on a non-object in sambaAccount
- fd-plugins#1662 Sections should not be used for anything but display
- fd-plugins#1674 error in sclaix plugin when generating the locales
- fd-plugins#1675 error in squiq plugin when generating the locales
- fd-plugins#1676 You are not allowed to view the fai summary.
- fd-plugins#1679 when creating a new server : Indirect modification of overloaded property serverGeneric::$cn
- fd-plugins#1680 when creating a fai profile PHP error: Undefined variable: name
- fd-plugins#1681 when creating an application : PHP error: Undefined property: application::$gotoLogoffScript
- fd-plugins#1682 when creatin a samba user with the sid inside the ldap PHP error: Undefined variable: sambaSID
- fd-plugins#1691 on the main page with the cyrus plugin installed PHP error: Missing argument 4 for IntAttribute::__construct(), called in
- fd-plugins#1694 in the apache plugin i have a back button but no save button on the list of virtual hosts
- fd-plugins#1703 Clean up the DaemonEvent mess
- fd-plugins#1716 duplicate ip with different mac in same dhcp section
- fd-plugins#1730 winstation network info are not saved
- fd-plugins#1751 error when modify a user
- fd-plugins#1762 Plugins configuration files not stored in /etc
- fd-plugins#1767 they should be a default domain if none is present in the ldap tree
- fd-plugins#1778 unable to see all supannStructures
- fd-plugins#1779 when creating a user, template aren't taken correctly
- fd-plugins#1821 schema dependencies
- fd-plugins#1834 argonaut seems broken ?
- fd-plugins#1847 syntax error, unexpected ')' in asterisk plugin
- fd-plugins#1851 Systems filter checkbox are always there
- fd-plugins#1856 check all the plugin for ("") in the code
- fd-plugins#1860 Cleaning all the schema in the system plugin
- fd-plugins#1883 DNS is_this_account does not work
- fd-plugins#1886 when saving a user I get this error
- fd-plugins#1897 error when loading quota plugin
- fd-plugins#1902 link to images is wrong in the asterisk plugin
- fd-plugins#1903 link to images is wrong in the fax plugin
- fd-plugins#1925 Cyrus connect bug?
- fd-plugins#1928 verify the description into the schema there is lot of copy paste
- fd-plugins#1944 error in quota plugin when generating the locales
- fd-plugins#1945 error in samba plugin when generating the locales
- fd-plugins#1946 french language in the supann plugin
- fd-plugins#1952 error in supann plugin when generating the locales
- fd-plugins#1970 Encodings config field mask needs to be fixed
- fd-plugins#1971 translation defaut nisGroup tabs name
- fd-plugins#1974 Supann tabs appears twice
- fd-plugins#1976 missing objectClass fdOpsiConf
- fd-plugins#1977 SUPANN can't creat an etablissement
- fd-plugins#1990 There are some mistakes in supportDaemon and DaemonEvent code
- fd-plugins#2002 inherited attribute is missing
- fd-plugins#2024 duplicate objectClass: "1.3.6.1.4.1.38414.20.2.3" in opsi-fd schema
- fd-plugins#2025 when adding an apsi profiles I get structural error
- fd-plugins#2028 bad description in opsi-fd.schema
- fd-plugins#2030 board plugin is broken
- fd-plugins#2034 when removing a profile i doesnt show the name of the profile between ()
- fd-plugins#2035 when copy/pasting a disk in fai i got this error
- fd-plugins#2064 the name of the product is not right in opsi products selections in the profile
- fd-plugins#2102 update the copyright in all plugins
- fd-plugins#2104 The order of acl constructor attributes is wrong in some places
- fd-plugins#2106 redeclaration of mail attribute in zarafa/class_groupMail.inc
- fd-plugins#2107 when I remove FAI options in dFD I geta n Objetc class violations
- fd-plugins#2114 check that the freeradius plugin is working correctly
- fd-plugins#2130 expiration date doesn't appear
- fd-plugins#2135 PHP error: Indirect modification of overloaded property mailAccount::$mail  has no effect
- fd-plugins#2139 Check foreach code
- fd-plugins#2141 Can't delete Samba settings from user object
- fd-plugins#2178 localization error in zarafa plugin
- fd-plugins#2182 size of windows for managing properties of opsi products
- fd-plugins#2198 second parameter missing in the freeradius/personnal/main.inc
- fd-plugins#2204 Cyrus mailboxes not being created
- fd-plugins#2205 Error installing rsyslog plugin
- fd-plugins#2208 sieve setting are saved but not showed back when reopng it
- fd-plugins#2210 apache plugin error with server alias check should be implemented to validate data entry
- fd-plugins#2222 Can't delete fai tab
- fd-plugins#2223 DNS service is triggering PHP errors
- fd-plugins#2232 ldap2repository doesn't exist, but is part of config dialog
- fd-plugins#2233 error in user with terminal server in samba account activated
- fd-plugins#2241 Missing base selector in sympaAlias
- fd-plugins#2243 wrong title for label fdDSARDN
- fd-plugins#2244 no title for some labels in user mail tab
- fd-plugins#2246 wrong title for label fdCyrusUseSlashes
- fd-plugins#2247 wrong title for label fdCyrusUseSlashes
- fd-plugins#2249 required field not editable in postfix service
- fd-plugins#2250 syntax error in mail plugin
- fd-plugins#2255 wrong title and label for radiusFramedIPAddress
- fd-plugins#2259 remove dorkboy from freeradius plugin
- fd-plugins#2262 Fatal error: cannot instantiate class 'String' - try running  'fusiondirectory-setup --update-cache' to fix this ?
- fd-plugins#2270 Lots of errors and problems in the sudo plugin
- fd-plugins#2275 user miniicon  don't appear in sudo-plugin
- fd-plugins#2276 Argonaut actions are called twice
- fd-plugins#2280 Error: No such object in netgroups plugin
- fd-plugins#2284 we need a corrected version of the netways.schema because only one structural classe is allowed
- fd-plugins#2292 error in opsi import
- fd-plugins#2300 the miniIcon is wrong in sudo list
- fd-plugins#2314 Translation are wrong in SUPANN
- fd-plugins#2321 OPSI remove_from_parent is wrong
- fd-plugins#2323 the add ldap serveur is always ghosted and doesnt show the ldap uri
- fd-plugins#2327 strange errors when apply in opsiClient
- fd-plugins#2329 call OPSI.update_or_insert in both cases (opsi client in winstation or opsi client in objectgroup)

### Security

#### fusiondirectory
- fd#2062 the template don't escape the html inside itself

#### fusiondirectory-plugins
- fd-plugins#2140 the template don't escape the html inside itself
