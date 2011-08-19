#!/usr/bin/perl

use strict;
use warnings;
use 5.010;

# used to identify linux distro
use Linux::Distribution qw(distribution_name distribution_version);

# allow use of "switch"
use Switch;

# used to manage files
use Path::Class;

# USE FOR DEBUG, REMOVE IT WHEN THE SCRIPT IS DONE
use Data::Dumper;

# try to determinate on wich linux distro this script run
my $linux = Linux::Distribution->new;
my $distro = $linux->distribution_name();

# fd's directory and class.cache file's path declaration
my $fd_home = "";
my $fd_cache = "";

# set location variables according to distro
switch( $distro ) {
	
	case "debian" { $fd_home = "/usr/share/fusiondirectory/", $fd_cache = "/var/cache/fusiondirectory/" } 
	else { die ( "\nUnknown distro\n" ) }
}
 

my $locale_dir = $fd_home."locale";
my $state_dir = $fd_home."state";
my $class_cache = $fd_cache."class.cache";
my $locale_cache_dir = $fd_cache."locale";

###################################################################################################################################################

sub user_is_root {

	# return true if the script run as root
	if ( $ENV{USER} eq "root" ) {

		return 1;

	} else {

		return 0;
	}
}

####################################################### class.cache update #########################################################################


# hash that will contain the result of the "get_classes" function
my %get_classes_result = ();

# reference on the "get_classes_result" hash
my $ref_get_classes_result = \%get_classes_result;

# function that scan recursivly a directory to find .inc and . php
# then return a hash with class => path to the class file
sub get_classes {

	my ( $path, $ref_result ) = @_;

	# if this function has been called with a parameter
	if ( defined ( $path ) ) {

		# create a "dir" object
		my $dir = dir ( $path );

		# create an array with the content of $dir
		my @dir_files = $dir->children;

		foreach my $file ( @dir_files ) {

			# recursive call if $file is a directory
			if ( -d $file ) {

				get_classes ( $file, $ref_result );
				next;
			}

			# only process if $file is a .inc or a .php file
			if ( ( $file =~ /.*\.(inc|php)$/ ) && ( $file !~ /.*smarty.*/ ) ) {

				# put the entire content of the file pointed by $file in $data
				my @lines = $file->slurp;

				foreach my $line ( @lines ) {

					# remove \n from the end of each line
					chomp $line;

					# only process for lines beginning with "class", and extracting the 2nd word (the class name)
					if ( $line =~ /^class\s*(\w+).*/ ) {

						my $class = $1;

						# modifing $file, to contains relative path, not complete one
						$file =~ s/^$fd_home\/(\S+)/$1/;

						# adding the values (class name and file path) to the hash
						$ref_result->{$class} = $file;

					# else, go to the next line
					} else {
						next;
					}
				}

			# else process with the next file
			} else {

				next;
			}
		}

	# if this function has been called without parameter
	} else {

		die ( "function get_classes called without parameter" );
	}

}





sub rescan_classes {

	say ( "\nUpdating class cache\n" );

	get_classes ( $fd_home, $ref_get_classes_result );

	# create a "file" object with the $class_cache path
	my $file_class = file ($class_cache);

	# create the handler (write mode) for the file previoulsy created
	my $fhw = $file_class->openw() or die ( "\nUnable to open $class_cache in read mode" );	

	# first lines of class.cache
	$fhw->print ( "<?php\n\t\$class_mapping= array(\n");

	# for each $key/$value, writting a new line to $class_cache
	while ( my ( $key,$value ) = each %get_classes_result ) {

		$fhw->print ( "\t\t$key => $value\n" );

	}

	# last line of classe.cache
	$fhw->print ( "\t);\n?>" );

	$fhw->close;
}


###################################################### Internalisation's update ####################################################################################

# hash that will contain the result of the "get_classes" function
my %get_i18n_result = ();

# reference on the "get_classes_result" hash
my $ref_get_i18n_result = \%get_i18n_result;


sub get_i18n {

	my ( $path, $ref_result ) = @_;

	# if this function has been called with a parameter
	if ( defined ( $path ) ) {

		# create a "dir" object
		my $dir = dir ( $path );

		# create an array with the content of $dir
		my @dir_files = $dir->children;

		foreach my $file ( @dir_files ) {

			# recursive call if $file is a directory
			if ( -d $file ) {

				get_i18n ( $file, $ref_result );
				next;
			}

			# if the file's directory is ???/LC_MESSAGES/messages.po
			if ( $file =~ /^.*LC_MESSAGES\/messages.po$/ ) {

				# language recuperation (fr/en/es/it...)
				$file =~ /^.*\/(\w+)\/LC_MESSAGES\/messages.po$/;
				my $lang = $1;

				# push the file's path in the $lang array (wich is inside the hash pointed by $ref_result
				push @{$ref_result->{$lang}}, $file;

			} else {

				next;
			}

		}

	# if this function has been called without parameter
	} else {

		die ( "function get_classes called without parameter" );
	}

	
}





sub rescan_i18n {

	say ( "\nUpdating internalization\n" );

	get_i18n ( $locale_dir, $ref_get_i18n_result );

	while ( my ( $lang, $files ) = each %get_i18n_result ) {

		# directory wich will contain the .mo file for each language
		my $lang_cache_dir = dir ( "$locale_cache_dir/$lang/LC_MESSAGES" );
 
		# if $lang_cache_dir doesn't already exists, creating it
		if ( !-d $lang_cache_dir ) {

			$lang_cache_dir->mkpath;
		}

		# glue .po files's names
		my $po_files = join(" ", @{$files});
		chomp $po_files;

		# merging .po files
		system ( "msgcat --use-first ".$po_files.">".$lang_cache_dir."/messages.po" ) and die ( "Unable to merge .po files for $lang\n" );

		# compiling .po files in .mo files
		system ( "msgfmt -o $lang_cache_dir/messages.mo $lang_cache_dir/messages.po && rm $lang_cache_dir/messages.po" ) and die ( "Unable to compile .mo files\n" );


	}

	say ( "! Warning: you may need to reload your webservice!\n" );

}







#####################################################################################################################################################################


if ( user_is_root() ) {

	rescan_classes();
	rescan_i18n();

} else {

	say ( "\nYou have to run this script as root" );
	exit -1;
} 
