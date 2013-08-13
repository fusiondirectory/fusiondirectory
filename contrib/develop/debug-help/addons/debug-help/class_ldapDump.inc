<?php

class ldapDump extends simplePlugin
{
  static function plInfo()
  {
    return array(
      'plShortName'   => _('LDAP'),
      'plDescription' => _('LDAP Dump'),
      'plObjectType'  => array('special'),
      'plPriority'    => 99,

      'plProvidedAcls' => parent::generatePlProvidedAcls(self::getAttributesInfo())
    );
  }

  static function getAttributesInfo ()
  {
    return array(
      'main' => array(
        'name'      => _('Diagrams'),
        'class'     => array('fullwidth'),
        'attrs'     => array(
          new FakeAttribute('dump')
        ),
        'template'  => get_template_path('ldapdump.tpl', TRUE, dirname(__FILE__))
      ),
    );
  }

  function __construct(&$config, $dn = NULL, $object = NULL)
  {
    parent::__construct($config, $dn, $object);

    $this->dump = NULL;
  }

  function execute()
  {
    if ($this->dump === NULL) {
      $ui = get_userinfo();
      if (preg_match('/r/', $ui->get_category_permissions($this->acl_base, 'all'))) {
        $ldap = $this->config->get_ldap_link();
        $ldap->cat($this->dn);
        if ($attrs = $ldap->fetch()) {
          $this->dump = array($this->dumpLdapNode($attrs, $this->config->current['BASE']));
        } else {
          $this->dump = array();
        }
      } else {
        $this->dump = FALSE;
      }
    }
    $smarty = get_smarty();
    $smarty->assign('usePrototype', 'true');
    return parent::execute();
  }

  function dumpLdapNode($attrs, $parent_dn = '')
  {
    $node = array(
      'dn'        => $attrs['dn'],
      'name'      => preg_replace('/,'.preg_quote($parent_dn).'$/', '', $attrs['dn']),
      'attrs'     => array(),
      'subnodes'  => array(),
    );
    for ($i = 0; $i < $attrs['count']; $i++) {
      $key                  = $attrs[$i];
      $node['attrs'][$key]  = $attrs[$key];
      unset($node['attrs'][$key]['count']);
    }

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($attrs['dn']);
    $ldap->ls();
    while ($attrs = $ldap->fetch()) {
      $node['subnodes'][$attrs['dn']] = $this->dumpLdapNode($attrs, $node['dn']);
    }

    return $node;
  }
}

?>
