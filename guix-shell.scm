#!/usr/bin/env sh
exec guix shell -m $0 -- kicad # Opens recommended KiCAD in a reproducible environment 
!#
;;; TERES-1 - The DIY Notebook kit by OLIMEX, Ltd.
;;; Copyright (C) 2022 Jacob Hrbek <kreyren@rixotstudio.cz>
;;;
;;; This file is Free/Libre Open-Source Software; you may copy, redistribute and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version.
;;; This file is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public license along with this project. If not, see <http://www.gnu.org/licenses>

(use-modules
	(guix channels))

;;; Commentary:
;;;
;;; This file is used to provide a standard environment for developers using GNU Guix
;;;
;;; On systems outside of GNU GuixSD (the standalone distribution) you can use this deployment by installing 'guix' (e.g. `apt-get install guix --yes`) which should be packaged by your GNU distribution and then:
;;;
;;;    1. Run `$ guix pull # To update the derivations ensuring that you get up-to-date recipes`
;;;    2. Executing this file directly through provided shebang or by using:
;;;        `$ guix shell -m path/to/this/file -- kicad # Opens recommended KiCAD`
;;;
;;; Code:

;; NOTE(Krey): This can be used to pin specific release of guix to enforce version reproducibility, kept here in case it will be needed in the future
; (list (channel
;        (name 'guix)
;         (url "https://git.savannah.gnu.org/git/guix.git")
;         (commit
;           "f1bfd9f1948a5ff336d737c0614b9a30c2bb3097")
;        (introduction
;           (make-channel-introduction
;             "9edb3f66fd807b096b48283debdcddccfea34bad"
;             (openpgp-fingerprint
;               "BBB0 2DDF 2CEA F6A8 0D1D  E643 A2A0 6DF2 A33A 54FA")))))

(specifications->manifest (list "kicad@6.0.7"))

;;; guix-shell.scm ends here
