      'demo' => array(
        'name'  => _('Demo'),
        'attrs' => array(
          new StringAttribute (
            _('StringAttribute'),             // Label
            _('This is a string attribute'),  // Description
            'fakeString',                     // LDAP name
            FALSE,                             // Mandatory
            'example value'                   // Default value
          ),
          new PasswordAttribute (
            _('PasswordAttribute'),
            _('This is a password attribute'),
            'fakePassword',
            FALSE,
            'example value'
          ),
          new IntAttribute (
            _('IntAttribute'),
            _('This is an int attribute'),
            'fakeInt',
            FALSE,
            0,
            1337,
            42
          ),
          new FloatAttribute (
            _('FloatAttribute'),
            _('This is an float attribute'),
            'fakeFloat',
            FALSE,
            0.0,
            1337.0,
            13.37
          ),
          new SelectAttribute (
            _('SelectAttribute'),
            _('This is a select attribute'),
            'fakeSelect',
            FALSE,
            array('choice1', 'choice2'),
            'choice1',
            array('Choice 1', 'Choice 2')
          ),
          new BooleanAttribute (
            _('BooleanAttribute'),
            _('This is a boolean attribute'),
            'fakeBoolean',
            FALSE,
            FALSE
          ),
          new FileAttribute (
            _('FileAttribute'),
            _('This is a file attribute'),
            'fakeFile',
            FALSE
          ),
          new DateAttribute (
            _('DateAttribute'),
            _('This is a date attribute'),
            'fakeDate',
            FALSE,
            'd.m.Y'
          ),
          new BaseSelectorAttribute (
            'ou=fake'
          ),
          new SetAttribute (
            new StringAttribute (
              _('SetAttribute'),
              _('This is a set attribute of strings'),
              'fakeSet',
              FALSE
            ),
            array(
              'example value1',
              'example value2',
            )
          ),
          new OrderedArrayAttribute (
            new CompositeAttribute (
              _('This is an OrderedArrayAttribute of composite attributes'),
              'fakeComposite',
              array(
                new StringAttribute (
                  _('StringAttribute'),
                  _('This is a set attribute of strings'),
                  'fakeCompositeString',
                  FALSE
                ),
                new SelectAttribute (
                  _('SelectAttribute'),
                  _('This is a select attribute'),
                  'fakeCompositeSelect',
                  TRUE,
                  array('choice1', 'choice2'),
                  'choice1',
                  array('Choice 1', 'Choice 2')
                )
              ),
              '/^(.+):(.+)$/',
              '%s:%s'
            ),
            TRUE,
            array(
              'value1:choice1',
              'value1:choice2',
              'value2:choice1'
            )
          ),
          new UsersAttribute (
            _('UsersAttribute'),
            _('This is a users attribute'),
            'fakeUsers',
            FALSE
          ),
        )
      ),
