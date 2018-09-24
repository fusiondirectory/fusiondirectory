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

