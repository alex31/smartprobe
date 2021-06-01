#!/usr/bin/perl 

#
#
#

use strict;
use warnings;
use feature ':5.24';
use Device::SerialPort;
use Modern::Perl '2016';
use Carp qw/longmess cluck confess/;
use POSIX;


no warnings 'experimental::smartmatch';

sub fletcher16 ($$);
sub statusFunc ($$$$$$$);
sub initSerial ($);
sub serialCb();
sub receiveMessageCB($);
sub telemetryPingCB();
sub getSerial();
sub fhbits(@);

my %options;


my @devStateLabel = (undef, undef);


my $serialName = $ARGV[0] // "/dev/ttyACM2";
my $baudRate =   $ARGV[1] // 230400;

say "baud rate is $baudRate";

$serialName = getSerial () if $serialName eq "/dev/ttyACMx";

say "serialName = $serialName";

if ($serialName ne '§') {
    die "cannot open $serialName $!\n" unless -e $serialName;
}

my $serial;

do {
    $serial = initSerial ($serialName);
    sleep (1) unless $serial;
} unless defined $serial;

while (1) {
    serialCb();
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
	say '-------------------------------';
	say sprintf("acc = [%.2f, %.2f, %.2f]", @pl[0..2]);
	say sprintf("gyr = [%.2f, %.2f, %.2f]", @pl[3..5]);
	say sprintf("att = [%.2f, %.2f, %.2f, %.2f]", @pl[6..9]);
	say sprintf("tas=%.2f eas=%.2f, alpha=%.2f, beta=%.2f", @pl[10..13]);
	say sprintf("pre=%.2f tem=%.2f, rho=%.2f", @pl[14..16]);
	say sprintf("DC=[%.2f, %.2f]; DH=[%.2f, %.2f]; DV=[%.2f, %.2f]", @pl[17..22]);
    } else {
	perror "message error unknown id $id";
    }
}





