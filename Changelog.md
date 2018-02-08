## %"FusionDirectory 1.2.1" - 2018-02-08

### Added

#### fusiondirectory
- fd#5342 SimplePlugin should have a way to say a tab cannot be created for some reason
- fd#5694 Cannot add a group to "users and groups" field in sudo role
- fd#5703 Sort entries in users dashboard
- fd#5746 Using date modifier does not work for unix password expiration

#### fusiondirectory-plugins
- fd-plugins#5548 Sort entries in ppolicy dashboard

### Changed

#### fusiondirectory
- fd#5668 Password length on login page should have no limit
- fd#5692 Update the readme.md with correct forge information
- fd#5737 Dashboard tabs should be ordered
- fd#5749 Webservice errors

#### fusiondirectory-plugins
- fd-plugins#5685 supannRefId on Structure object is too small and mono valued
- fd-plugins#5688 Backport the DNS code from 1.3-dev for speed and usability reason
- fd-plugins#5713 Phones have wrong attribute order
- fd-plugins#5716 DNS Zone edition is too slow
- fd-plugins#5740 Dashboard tabs are in random order

### Fixed

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
- fd#5758 Error when renaming a object with single-value main attribute
- fd#5757 In case of LDAP errors during a deletion, a PHP error is triggered

#### fusiondirectory-plugins
- fd-plugins#4036 Webservice does not display ACL errors in some cases
- fd-plugins#4348 Fusion inventory should return an error if schema is missing
- fd-plugins#5629 Error message when removing some content from partage tab
- fd-plugins#5672 Mail plugin incompatible with PHP 7.1
- fd-plugins#5673 PHP error on login page when applications plugin is installed and applications in menu set to only allowed
- fd-plugins#5707 posixAccount foreignKey is wrong
- fd-plugins#5708 software list and profile management error when using several OPSI Servers
- fd-plugins#5709 DNS Tab - record filtering is broken for new systems
- fd-plugins#5724 SOA serial is rewriten to each reverse even if no modification is made to any record related to these domains
- fd-plugins#5725 Crash when I remove IP from a workstation with a DNS record
- fd-plugins#5729 SOA is not updated in reverse zones if there is no record change
- fd-plugins#5745 PHP error in OPSI product properties
- fd-plugins#5747 Line number in CSV import error messages is incorrect
- fd-plugins#5754 Forward confirmation to manager does not work when extending the account expiration
- fd-plugins#5755 User plugin reminder not support accent in message
- fd-plugins#5757 DHCP: Changing mac address in the system's system tab does not update the DHCP entries accordingly
- fd-plugins#5766 Crash when DHCP plugin is not installed when we use OPSI import
- fd-plugins#5768 Renaming DNS zones often results in an Exception being thrown

