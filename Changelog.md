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

