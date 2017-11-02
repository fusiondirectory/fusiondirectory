## %"FusionDirectory 1.2.1" - 2017-11-02

### Added

#### fusiondirectory
- fd#5703 Sort entries in users dashboard

#### fusiondirectory-plugins
- fd-plugins#5548 Sort entries in ppolicy dashboard

### Changed

#### fusiondirectory-plugins
- fd-plugins#5685 supannRefId on Structure object is too small and mono valued
- fd-plugins#5713 Phones have wrong attribute order

### Fixed

#### fusiondirectory
- fd#5348 LDIF import should support folded comments
- fd#5588 fusiondirectory-setup custom smarty directory forced to lowercase
- fd#5589 SASL missing from Password default hash field in setup wizard
- fd#5668 Password length on login page should have no limit
- fd#5701 Management filter is broken when mainAttr is FALSE
- fd#5694 Cannot add a group to "users and groups" field in sudo role
- fd#5696 If tab objectClass contains top, tab gets activated by default
- fd#5700 Non-existing classes in pluglist::$info
- fd#5711 ACL list have a PHP error when there are ACL on objects
- fd#5712 Error text is wrong is some cases
- fd#5724 Error before login on Centos 7
- fd#5726 Log to fd behind an lemonldap::ng portal show debug log instead of main page

#### fusiondirectory-plugins
- fd-plugins#5629 when remove some content on partage tab, an error occurs
- fd-plugins#5672 Mail plugin incompatible with PHP 7.1
- fd-plugins#5673 PHP error on login page when applications plugin is installed and applications in menu set to only allowed
- fd-plugins#5688 backport the dns code from 1.3-dev for speed and usability reason
- fd-plugins#5707 posixAccount foreignKey is wrong
- fd-plugins#5708 software list and profile management error when using several OPSI Servers
- fd-plugins#5709 DNS Tab - record filtering is broken for new systems
- fd-plugins#5716 DNS Zone edition is too slow

