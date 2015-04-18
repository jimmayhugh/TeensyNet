-- phpMyAdmin SQL Dump
-- version 3.4.11.1deb2+deb7u1
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: Apr 18, 2015 at 09:40 AM
-- Server version: 5.5.41
-- PHP Version: 5.4.39-0+deb7u2

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `teensynet`
--
DROP DATABASE IF EXISTS `teensynet`;
CREATE DATABASE `teensynet` DEFAULT CHARACTER SET latin1 COLLATE latin1_swedish_ci;
USE `teensynet`;

-- --------------------------------------------------------

--
-- Table structure for table `action`
--

DROP TABLE IF EXISTS `action`;
CREATE TABLE IF NOT EXISTS `action` (
  `id` int(11) NOT NULL,
  `active` tinyint(1) NOT NULL,
  `tempAddr` varchar(50) NOT NULL,
  `tcAddr` varchar(50) NOT NULL,
  `tcTrigger` int(11) NOT NULL,
  `tcDelay` bigint(20) NOT NULL,
  `thAddr` varchar(50) NOT NULL,
  `thTrigger` int(11) NOT NULL,
  `thDelay` bigint(20) NOT NULL,
  `lcd` tinyint(4) NOT NULL,
  `lcd1w` varchar(20) NOT NULL,
  `glcd` varchar(30) NOT NULL,
  `netID` int(11) NOT NULL,
  KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `actionGraph`
--

DROP TABLE IF EXISTS `actionGraph`;
CREATE TABLE IF NOT EXISTS `actionGraph` (
  `id` int(11) NOT NULL,
  `time` bigint(20) NOT NULL,
  `temp` int(11) NOT NULL,
  `tcTemp` int(11) NOT NULL,
  `tcSwitch` enum('ON','OFF','NONE') NOT NULL,
  `thTemp` int(11) NOT NULL,
  `thSwitch` enum('ON','OFF','NONE') NOT NULL,
  `netID` int(1) NOT NULL,
  `netName` varchar(25) NOT NULL,
  KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `chipNames`
--

DROP TABLE IF EXISTS `chipNames`;
CREATE TABLE IF NOT EXISTS `chipNames` (
  `id` int(1) NOT NULL,
  `netID` int(1) NOT NULL,
  `address` varchar(50) NOT NULL,
  `name` varchar(16) NOT NULL,
  `lcd` tinyint(4) NOT NULL,
  `lcd1w` varchar(20) NOT NULL,
  `glcd` varchar(30) NOT NULL,
  `IndexKey` int(11) NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`IndexKey`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=380 ;

-- --------------------------------------------------------

--
-- Table structure for table `glcdNames`
--

DROP TABLE IF EXISTS `glcdNames`;
CREATE TABLE IF NOT EXISTS `glcdNames` (
  `id` int(1) NOT NULL,
  `netID` int(1) NOT NULL,
  `address` varchar(50) NOT NULL,
  `name` varchar(16) NOT NULL,
  `IndexKey` int(11) NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`IndexKey`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `lcdNames`
--

DROP TABLE IF EXISTS `lcdNames`;
CREATE TABLE IF NOT EXISTS `lcdNames` (
  `id` int(1) NOT NULL,
  `netID` int(1) NOT NULL,
  `address` varchar(50) NOT NULL,
  `name` varchar(16) NOT NULL,
  `IndexKey` int(11) NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`IndexKey`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Table structure for table `netDevices`
--

DROP TABLE IF EXISTS `netDevices`;
CREATE TABLE IF NOT EXISTS `netDevices` (
  `netID` int(11) NOT NULL AUTO_INCREMENT,
  `netName` varchar(50) NOT NULL,
  `service_port` varchar(20) NOT NULL,
  `port_address` smallint(6) NOT NULL,
  `netActive` tinyint(1) NOT NULL,
  UNIQUE KEY `netID` (`netID`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=106 ;

-- --------------------------------------------------------

--
-- Table structure for table `pid`
--

DROP TABLE IF EXISTS `pid`;
CREATE TABLE IF NOT EXISTS `pid` (
  `id` int(11) NOT NULL,
  `enabled` tinyint(1) NOT NULL DEFAULT '0',
  `tempAddr` varchar(50) NOT NULL DEFAULT '0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00',
  `setpoint` double NOT NULL,
  `switchAddr` varchar(50) NOT NULL DEFAULT '0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00',
  `kp` double NOT NULL DEFAULT '0',
  `ki` double NOT NULL DEFAULT '0',
  `kd` double NOT NULL DEFAULT '0',
  `direction` int(11) NOT NULL DEFAULT '1',
  `windowSize` int(11) NOT NULL DEFAULT '5000',
  `netID` int(1) NOT NULL,
  KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `pidGraph`
--

DROP TABLE IF EXISTS `pidGraph`;
CREATE TABLE IF NOT EXISTS `pidGraph` (
  `id` int(11) NOT NULL,
  `setPoint` int(1) NOT NULL,
  `temp` int(11) NOT NULL,
  `switch` tinyint(1) NOT NULL,
  `direction` tinyint(1) NOT NULL,
  `time` bigint(20) NOT NULL,
  `netID` int(1) NOT NULL,
  `netName` varchar(25) NOT NULL,
  KEY `id` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
--
-- Database: `test`
--
CREATE DATABASE `test` DEFAULT CHARACTER SET latin1 COLLATE latin1_swedish_ci;
USE `test`;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
