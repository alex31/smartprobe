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
sub fletcher16 ($$);
sub statusFunc ($$$$$$$);
sub receiveMessageCB($);
sub telemetryPingCB();
sub fhbits(@);


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
    'dpScale' => 1,
    'rho' => 0,
    'velo' => 0,
    'alpha' => 0,
    'beta' => 0
);



my ($logFd, $logFdCount);

my $serialName = $ARGV[0] // "/dev/ttyUSB0";
my $baudRate =   $ARGV[1] // 115200;

say "DBG baud rate is $baudRate";

$serialName = getSerial () if $serialName =~ "/dev/ttyACM";

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

    labelLabelFrame($infoFrame, "Rho = ", \ ($varDataIn{'rho'}), 'left', 8);
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
    $port->handshake("none");
    $port->buffers(1, 1); #1 byte or it buffers everything forever
    $port->write_settings           || undef $port; #set
    unless ($port)                  { die "couldn't write_settings"; }

    return $port;
}



# 995.25  26.06   0.0000  0.0000  0.0000  25.12   25.12   25.06   4.84    28.5
# 995.24  26.06   0.0000  0.0000  0.0000  25.19   25.08   25.11   4.86    28.5
# 995.26  26.06   0.0000  0.0000  0.0000  25.09   25.13   25.08   4.86    28.1
# 995.25  26.07   0.0000  0.0000  0.0000  25.14   25.14   25.10   4.86    28.8

# sub serialCb()
# {
#     my $line = readline($serialHandle);
#     $line =~ s/[\[\]]//g;
#     my ($bp, $bt, $rho, $dp1, $dp2, $dp3, $dt1, $dt2, $dt3, $roll, $pitch,
# 	$velo, $alpha, $beta, $vcc, $mcut) = split (/\s+/, $line);

    
#     return unless defined $mcut;
    
#     $varDataIn{'baroTemp'} = $bt;
#     $varDataIn{'vcc'} = $vcc;
#     $varDataIn{'dpTemp'}->[0] = $dt1;
#     $varDataIn{'dpTemp'}->[1] = $dt2;
#     $varDataIn{'dpTemp'}->[2] = $dt3;
#     $varDataIn{'rho'} = $rho;
#     $varDataIn{'roll'} = $roll;
#     $varDataIn{'pitch'} = $pitch;
#     $varDataIn{'velo'} = $velo;
#     $varDataIn{'alpha'} = $alpha;
#     $varDataIn{'beta'} = $beta;
#     $tkObject{'baropg'}->value($bp * 10);
#     $tkObject{'dppg'}->[0]->value($dp1 * $varDataIn{'dpScale'});
#     $tkObject{'dppg'}->[1]->value($dp2 * $varDataIn{'dpScale'});
#     $tkObject{'dppg'}->[2]->value($dp3 * $varDataIn{'dpScale'});
#     $mw->idletasks();
# #    print $line;
# }




use constant  WAIT_FOR_SYNC => 0;
use constant  WAIT_FOR_LEN => 1;
use constant  WAIT_FOR_PAYLOAD => 2;
use constant  WAIT_FOR_CHECKSUM => 3;

sub serialCb()
{
    state $state = WAIT_FOR_SYNC;
    state $sync = 0;
    state $buffer;
    state $len;
    state $crcBuf;
    state @crc;
    state $calculatedCrc;
    state $receivedCrc;
    my $totLen;

    for ($state) {
	when  (WAIT_FOR_SYNC) {
	    my $status = sysread (FHD, $buffer, 1);
	    # unless ($status == 1) {
	    # 	say "link is broken :$! aborting\n" ;
	    # 	exit (-1);
	    # }
	    if ($status == 1) {
		($sync) = unpack ('C', $buffer);
		if ($sync == 0x99) {
		    $state = WAIT_FOR_LEN;
		} 
	    }
	}
	
	when  (WAIT_FOR_LEN) {
	    sysread (FHD, $buffer, 1);
	    ($len) = unpack ('C', $buffer);
	    $len -= 4; # protocol
	    $state = WAIT_FOR_PAYLOAD;  
	}
	
	when  (WAIT_FOR_PAYLOAD) {
#	    say ("len is $len");
	    $totLen = 0;
	    do {
		$totLen += sysread (FHD, $buffer, $len-$totLen, $totLen);
	    } while ($totLen != $len);
	    
	    $receivedCrc = fletcher16 (\$buffer, undef);
	    $state = WAIT_FOR_CHECKSUM;
	}
	
	when  (WAIT_FOR_CHECKSUM) {
	    $totLen = 0;
	    do {
		$totLen += sysread (FHD, $crcBuf, 2-$totLen, $totLen);
	    } while ($totLen != 2);
	    
	    @crc = unpack ('CC', $crcBuf);
	    $calculatedCrc =  ($crc[1] << 8)  | $crc[0];
	    if ($calculatedCrc == $receivedCrc) {
		receiveMessageCB (\$buffer);
	    } else {
		printf ("CRC DIFFER C:0x%x != R:0x%x\n", $calculatedCrc, $receivedCrc);
		receiveMessageCB (\$buffer);
	    }
	    $state = WAIT_FOR_SYNC;
	    $buffer=undef;
	}
    }
    
}


sub fletcher16 ($$) 
{
    my ($bufferRef, $msgIdRef) = @_;
    my $index;
    
    my @buffer = unpack ('C*', $$bufferRef);
    my $count = scalar(@buffer);
#    say "DBG count = $count";
    
    my $sum1 = ($count+4) & 0xff;
    my $sum2 = $sum1;

    for($index = 0; $index < $count; $index++)  {
	#say "B[$index]=$buffer[$index]";
	$sum1 = ($sum1 + $buffer[$index]) & 0xff;
	$sum2 = ($sum2 + $sum1) & 0xff;
    }
    
    $$msgIdRef=$buffer[0] if defined $msgIdRef; # msgId
    return (($sum2 << 8) | $sum1);
}




sub getSerial()
{
    opendir (my $dhd, "/dev") || die "cannot opendir /dev\n";
    my @acm;

    while (my $fn = readdir ($dhd)) {
	push (@acm, $fn) if $fn =~ m|^stm_acm_\d+|;
    }
    closedir ($dhd);

    die "no ACM device\n" unless scalar @acm;
    return '/dev/' . (reverse sort (@acm))[0];
}




sub fhbits(@) 
{
    my @fhlist = @_;
    my $bits = "";
    for my $fh (@fhlist) {
	vec($bits, fileno($fh), 1) = 1;
    }
    return $bits;
}

sub waitabit()
{
    select(undef, undef, undef, 0.1);
}



sub receiveMessageCB ($)
{
    my ($bufferRef) = @_;
    state $lastPercent=0;
    
    my (@header) = unpack('C4', $$bufferRef);
    substr($$bufferRef, 0, 4, '');
    my $id = $header[3];
    
    if ($id == 60) {
	my (@pl) = unpack('f23', $$bufferRef);
	$pl[14] /= 100;
	# say '-------------------------------';
	# say sprintf("acc = [%.2f, %.2f, %.2f]", @pl[0..2]);
	# say sprintf("gyr = [%.2f, %.2f, %.2f]", @pl[3..5]);
	# say sprintf("att = [%.2f, %.2f, %.2f, %.2f]", @pl[6..9]);
	# say sprintf("tas=%.2f eas=%.2f, alpha=%.2f, beta=%.2f", @pl[10..13]);
	# say sprintf("pre=%.2f tem=%.2f, rho=%.2f", @pl[14..16]);
	# say sprintf("DC=[%.2f, %.2f]; DH=[%.2f, %.2f]; DV=[%.2f, %.2f]", @pl[17..22]);

	$varDataIn{'baroTemp'} = sprintf("%.2f", $pl[15]);
	$varDataIn{'dpTemp'}->[0] = sprintf("%.2f", $pl[18]);
	$varDataIn{'dpTemp'}->[1] = sprintf("%.2f", $pl[20]);
	$varDataIn{'dpTemp'}->[2] = sprintf("%.2f", $pl[22]);
	$varDataIn{'rho'} = sprintf("%.2f", $pl[16]);
	$varDataIn{'velo'} = sprintf("%.2f", $pl[10]);
	$varDataIn{'alpha'} = sprintf("%.2f", $pl[12]);
	$varDataIn{'beta'} = sprintf("%.2f", $pl[13]);
	$tkObject{'baropg'}->value($pl[14] * 10);
	$tkObject{'dppg'}->[0]->value($pl[17] * $varDataIn{'dpScale'});
	$tkObject{'dppg'}->[1]->value($pl[19] * $varDataIn{'dpScale'});
	$tkObject{'dppg'}->[2]->value($pl[21] * $varDataIn{'dpScale'});
	$mw->idletasks();

	
	
    } else {
	perror "message error unknown id $id";
    }
}





