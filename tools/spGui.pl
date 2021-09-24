#!/usr/bin/perl 

#
#
#

use strict;
use warnings;
use feature ':5.26';
use Device::SerialPort;
use Modern::Perl '2015';
use Tk;
use Tk::ProgressBar;
use Tk::LabFrame;
use Carp qw/longmess cluck confess/;
use POSIX;
use File::Temp qw/ :mktemp  /;


no warnings 'experimental::smartmatch';

sub initSerial ($);
sub serialCb();
sub getSerial();
sub generateGui();
sub generatePanel ();
sub generatePressTempFrame ($$$$$$$$);
sub labelLabelFrame ($$$$;$);


my $mw;
my $mwf;

my %options;
my %tkObject = (
    'baropg' => undef,
    'dppg' => [undef, undef, undef]
    );


my %varDataIn = (
    'dpPress' => [0,0,0],		
    'dpTemp' => [0,0,0],
    'baroPress' => 0,		
    'baroTemp' => 0,
    'vcc' => 0,
    'dpScale' => 1,
    'rho' => 0,
    'roll' => 0,
    'pitch' => 0,
    'velo' => 0,
    'alpha' => 0,
    'beta' => 0
);



my ($logFd, $logFdCount);

my $serialName = $ARGV[0] // "/dev/ttyACMx";
my $baudRate =   $ARGV[1] // 115200;

say "DBG baud rate is $baudRate";

$serialName = getSerial () if $serialName eq "/dev/ttyACMx";

say "DBG> serialName = $serialName";

my $serial;
do {
    my $serial = initSerial($serialName);
    sleep (1) unless $serial;
} unless defined $serial;

undef $serial;

open (my $serialHandle, "<", $serialName) || die "cannot open $serialName\n";
generateGui();

#$mw->repeat(100, sub {$mw->update()});
$mw->fileevent($serialHandle, 'readable', \&serialCb) ;
Tk::MainLoop;






#                 _____                  _    _______   _            
#                |  __ \                | |  |__   __| | |           
#                | |__) |   ___   _ __  | |     | |    | | _         
#                |  ___/   / _ \ | '__| | |     | |    | |/ /        
#                | |      |  __/ | |    | |     | |    |   <         
#                |_|       \___| |_|    |_|     |_|    |_|\_\        
sub generateGui()
{
    $mw = MainWindow->new;
    $mw->wm (title => "smart probe");
    my $w = $mw->screenwidth;
    my $h = $mw->screenheight;

    $mw->MoveToplevelWindow (0,0);

    $mwf =  $mw->Frame ()->pack(-side => 'left', -anchor => 'w');
    generatePanel ();
}


sub generatePanel ()
{
    my $outerFrame = $mwf->Frame ();
    $outerFrame->pack(-side => 'left', -anchor => 'w');
    
    my $baroFrame = $outerFrame->Frame (-bd => '5m', -relief => 'flat');
    $baroFrame->pack(-side => 'left', -anchor => 'w');

    my $diffPressFrame = $outerFrame->Frame (-bd => '5m', -relief => 'flat');
    $diffPressFrame->pack(-side => 'left', -anchor => 'w');

    my $infoFrame = $outerFrame->Frame (-bd => '5m', -relief => 'flat');
    $infoFrame->pack(-side => 'left', -anchor => 'w');

    generatePressTempFrame($baroFrame, "Baro",
			   \ ($varDataIn{'baroPress'}),
			   \ ($varDataIn{'baroTemp'}),
			   \ ($tkObject{'baropg'}),
			   9800, 10300, 'yellow'
	);

    for (my $i=0; $i<3; $i++) {
	generatePressTempFrame($diffPressFrame, "DiffPress",
			       \ ($varDataIn{'dpPress'}->[$i]),
			       \ ($varDataIn{'dpTemp'}->[$i]),
			       \ ($tkObject{'dppg'}->[$i]),
			       -1000, 1000, 'green'
	    );
	
    }

    $outerFrame->Scale (
        '-orient' => 'vertical', '-length' => 600, 
        '-from' => 100, '-to' => 1,
        '-resolution' => 1,
        '-variable' => \ ($varDataIn{'dpScale'}),
        '-background' => 'lightgreen',
        '-sliderlength' => 20,
        '-sliderrelief' => 'solid')->pack(-side => 'left', -anchor => 's', -expand => 'true');

    labelLabelFrame($infoFrame, "Vcc = ", \ ($varDataIn{'vcc'}), 'left', 6);
    labelLabelFrame($infoFrame, "Rho = ", \ ($varDataIn{'rho'}), 'left', 8);
    labelLabelFrame($infoFrame, "Roll = ", \ ($varDataIn{'roll'}), 'left', 6);
    labelLabelFrame($infoFrame, "Pitch = ", \ ($varDataIn{'pitch'}), 'left', 6);
    labelLabelFrame($infoFrame, "AirSpeed = ", \ ($varDataIn{'velo'}), 'left', 6);
    labelLabelFrame($infoFrame, "Alpha = ", \ ($varDataIn{'alpha'}), 'left', 6);
    labelLabelFrame($infoFrame, "Beta = ", \ ($varDataIn{'beta'}), 'left', 6);
}

sub generatePressTempFrame ($$$$$$$$) {
    my ($frame, $name, $pressRef, $tempRef, $pgRef, $from, $to, $color) =@_;
 
    my $dataFrame = $frame->Frame (-bd => '1m', -relief => 'sunken');
    $dataFrame->pack(-side => 'left', -anchor => 'w');
    labelLabelFrame($dataFrame, "T = ", $tempRef, 'left', 6);

    my $amp = $to -$from;
    my $redZone = $amp/20;

    $$pgRef = $dataFrame->ProgressBar(
	-width => 20,
	-length => 1000,
	-anchor => 's',
	-from => $from,
	-to => $to,
	-blocks => 20,
	-colors => [$from, 'red', $from+$redZone, $color, $to-$redZone, 'red']
	);
    $$pgRef->pack(-side => 'top', -anchor => 's');
 
}




sub labelLabelFrame ($$$$;$)
{
    my ($ef, $labelText, $textVar, $packDir, $width) = @_ ;
    
    my (
	$label,
	$entry,
	$frame,
	$frameDir
	) = ();
    
    $frameDir = ($packDir eq 'top') ? 'left' : 'top' ; 
    
    $width = 15 unless defined $width ;
    $frame = $ef->Frame ();
    $frame->pack (-side => $frameDir, -pady => '2m', -padx => '0m', 
		  -anchor => 'w', -fill => 'both', -expand => 'true');
    
    $label = $frame->Label (-text => $labelText);
    $label->pack (-side =>$packDir, -padx => '0m', -fill => 'y');
    
    $entry = $frame->Label (-width => $width, -relief => 'sunken',
			    -bd => 2, -textvariable => $textVar,
			    -font => "-adobe-courier-medium-r-*-*-14-*-*-*-*-*-iso8859-15") ;
    
    $entry->pack (-side =>'right', -padx => '0m', -anchor => 'e');
    
    return $entry ;
}


#                 ______                 _            _          
#                /  ____|               (_)          | |         
#                | (___     ___   _ __   _     __ _  | |         
#                 \___ \   / _ \ | '__| | |   / _` | | |         
#                .____) | |  __/ | |    | |  | (_| | | |         
#                \_____/   \___| |_|    |_|   \__,_| |_|         



sub initSerial ($)
{
    my $dev = shift;
 
    my $port = tie (*FHD, 'Device::SerialPort', $dev);
    unless ($port) {
	warn "Can't tie: $! .. still trying\n"; 
	return undef;
    }

#port configuration 115200/8/N/1
    $port->databits(8);
    $port->baudrate($baudRate);
    $port->parity("none");
    $port->stopbits(1);

    return $port;
}



sub getSerial()
{
    opendir (my $dhd, "/dev") || die "cannot opendir /dev\n";
    my @acm;

    while (my $fn = readdir ($dhd)) {
#	say ("DBG> fn = $fn");
	push (@acm, $fn) if $fn =~ m|^bmp_tty\d*|;
    }
    closedir ($dhd);

    die "no ACM device\n" unless scalar @acm;
    return '/dev/' . (reverse sort (@acm))[0];
}

# 995.25  26.06   0.0000  0.0000  0.0000  25.12   25.12   25.06   4.84    28.5
# 995.24  26.06   0.0000  0.0000  0.0000  25.19   25.08   25.11   4.86    28.5
# 995.26  26.06   0.0000  0.0000  0.0000  25.09   25.13   25.08   4.86    28.1
# 995.25  26.07   0.0000  0.0000  0.0000  25.14   25.14   25.10   4.86    28.8

sub serialCb()
{
    my $line = readline($serialHandle);
    $line =~ s/[\[\]]//g;
    my ($bp, $bt, $rho, $dp1, $dp2, $dp3, $dt1, $dt2, $dt3, $roll, $pitch,
	$velo, $alpha, $beta, $vcc, $mcut) = split (/\s+/, $line);

    
    return unless defined $mcut;
    
    $varDataIn{'baroTemp'} = $bt;
    $varDataIn{'vcc'} = $vcc;
    $varDataIn{'dpTemp'}->[0] = $dt1;
    $varDataIn{'dpTemp'}->[1] = $dt2;
    $varDataIn{'dpTemp'}->[2] = $dt3;
    $varDataIn{'rho'} = $rho;
    $varDataIn{'roll'} = $roll;
    $varDataIn{'pitch'} = $pitch;
    $varDataIn{'velo'} = $velo;
    $varDataIn{'alpha'} = $alpha;
    $varDataIn{'beta'} = $beta;
    $tkObject{'baropg'}->value($bp * 10);
    $tkObject{'dppg'}->[0]->value($dp1 * $varDataIn{'dpScale'});
    $tkObject{'dppg'}->[1]->value($dp2 * $varDataIn{'dpScale'});
    $tkObject{'dppg'}->[2]->value($dp3 * $varDataIn{'dpScale'});
    $mw->idletasks();
#    print $line;
}

