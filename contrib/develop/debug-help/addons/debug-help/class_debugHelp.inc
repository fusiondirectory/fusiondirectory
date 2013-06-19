<?php

class debugHelp
{
  static function plInfo()
  {
    return array(
      'plShortName'   => _('Diagram'),
      'plDescription' => _('Debug diagram help'),
      'plSection'     => 'addons',
      'plIcon'        => 'plugins/debug-help/images/icon.png',
      'plCategory'    => array('debug' => array('objectClass' => 'none', 'description' => _('Diagram'))),

      'plProvidedAcls' => array()
    );
  }

  /*!
   * \brief Build a diagram of object types and acl categories using graphviz
   */
  static function buildDiagram ()
  {
    global $config;
    $plist = session::global_get('plist');
    $acl_shape    = 'box';
    $obj_shape    = 'diamond';
    $class_shape  = 'ellipse';

    $nodes = '';
    $edges = '';
    foreach ($config->data['CATEGORIES'] as $key => $cat) {
      $label = $key;
      if (isset($cat['description'])) {
        $label = $cat['description'];
      }
      $nodes  .= "\t".'node [shape='.$acl_shape.',label="'.$label.'"] acl_'.$key.";\n";
    }
    foreach ($config->data['OBJECTS'] as $key => $obj) {
      $nodes  .= "\t".'node [shape='.$obj_shape.',label="'.$obj['name'].'"] obj_'.$key.";\n";
      $edges  .= "\t".'obj_'.$key.' -> acl_'.$obj['aclCategory']." [weight=2];\n";
    }
    foreach ($plist->info as $class => $infos) {
      if (isset($infos['plDepends'])) {
        foreach ($infos['plDepends'] as $dclass) {
          $edges .= "\t".'class_'.$class.' -> class_'.$dclass." [style=dotted];\n";
        }
      } elseif (!isset($infos['plObjectType']) || empty($infos['plObjectType'])) {
        continue;
      }
      if (is_subclass_of($class, 'simplePlugin')) {
        $color = 'green';
      } elseif (is_subclass_of($class, 'plugin')) {
        $color = 'red';
      } else {
        $color = 'yellow';
      }
      $nodes  .= "\t".'node [shape='.$class_shape.',label="'.$class.'",color='.$color.',style=filled] class_'.$class.";\n";
    }
    foreach ($config->data['TABS'] as $tabclass => $tabs) {
      $tabclass = preg_replace('/TABS$/', '', $tabclass);
      if (isset($config->data['OBJECTS'][$tabclass])) {
        $tabclass = 'obj_'.$tabclass;
      } else {
        $tabclass = 'tabclass_'.$tabclass;
        $nodes  .= "\t".'node [shape=plaintext,label="'.$tabclass.'",color=white] '.$tabclass.";\n";
      }
      foreach ($tabs as $tab) {
        if ($tabclass == 'tabclass_SERVERSERVICE') {
          if (is_subclass_of($tab['CLASS'], 'simpleService')) {
            $color = 'green';
          } elseif (is_subclass_of($tab['CLASS'], 'goService')) {
            $color = 'red';
          } else {
            $color = 'yellow';
          }
          $nodes .= "\t".'node [shape='.$class_shape.',label="'.$tab['CLASS'].'",color='.$color.',style=filled] class_'.$tab['CLASS'].";\n";
        }
        $edges .= "\t".'class_'.$tab['CLASS'].' -> '.$tabclass.";\n";
      }
    }
    $code = 'digraph objects {'."\n";
    $code .= $nodes;
    $code .= $edges;
    $code .= "\t".'overlap=false'.";\n";
    $code .= "\t".'splines=true'.";\n";
    $code .= "}\n";
    send_binary_content($code, 'fusiondirectory.gv');
  }
}

?>
