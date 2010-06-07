package L4::Grub;

use Exporter;
use vars qw(@ISA @EXPORT);
@ISA    = qw(Exporter);

use Getopt::Long;

sub parse_gengrub_args()
{
  my %a = ( timeout => undef,
            serial  => undef
	   );
  my @opts = ("timeout=i", \$a{timeout},
              "serial",    \$a{serial});

  if (!GetOptions(@opts))
    {
      print "Command line parsing failed.\n";
    }

  if (0)
    {
      print "Options:\n";
      print "timeout: $a{timeout}\n" if defined $a{timeout};
      print "serial : $a{serial}\n" if defined $a{serial};
    }

  %a;
}

sub prepare_grub1_dir($)
{
  my $dir = shift;

  return if -e "$dir/boot/grub/stage2_eltorito";

  my $copypath;
  my @grub_path = ("/usr/lib/grub/i386-pc", "/usr/share/grub/i386-pc",
                   "/boot/grub", "/usr/local/lib/grub/i386-pc",
                   "/usr/lib/grub/x86_64-pc");
  unshift @grub_path, $ENV{GRUB_PATH} if defined $ENV{GRUB_PATH};

  foreach my $p (@grub_path) {
    $copypath=$p if -e "$p/stage2_eltorito";
  }
  die "Cannot find a stage2_eltorito file..." unless defined $copypath;

  # copy files
  mkdir "$dir/boot";
  mkdir "$dir/boot/grub";
  system("cp $copypath/stage2_eltorito $dir/boot/grub");
  chmod 0644, "$dir/boot/grub/stage2_eltorito";
}

sub grub1_mkisofs($$@)
{
  my ($isofilename, $dir, @morefiles) = @_;
  system("cp -v ".join(' ', @morefiles)." $dir") if @morefiles;
  my $cmd = "mkisofs -f -R -b boot/grub/stage2_eltorito".
            " -no-emul-boot -boot-load-size 4 -boot-info-table".
            " -hide-rr-moved -J -joliet-long -o \"$isofilename\" \"$dir\"";

  print "Generating GRUB1 image with cmd: $cmd\n";
  system("$cmd");
  die "Failed to create ISO" if $?;
}

sub prepare_grub2_dir($)
{
  my $dir = shift;
  mkdir "$dir/boot";
  mkdir "$dir/boot/grub";
}

sub grub2_mkisofs($$@)
{
  my ($isofilename, $dir, @morefiles) = @_;
  my $cmd = "grub_mkisofs_arguments=-f grub-mkrescue"
            ." --output=\"$isofilename\" $dir ".join(' ', @morefiles);
  system("$cmd");
  die "Failed to create ISO" if $?;
}
