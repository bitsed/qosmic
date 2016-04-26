# Copyright 1999-2008 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit eutils

DESCRIPTION="Tools and a library for creating flame fractal images"
HOMEPAGE="http://flam3.com/"
SRC_URI="${HOMEPAGE}/${P}.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~amd64"
IUSE=""

DEPEND="dev-libs/libxml2
	media-libs/libpng
	media-libs/jpeg
	!<=x11-misc/electricsheep-2.6.8-r1"
RDEPEND="${DEPEND}"

src_install() {
	emake install DESTDIR="${D}" || die "emake install failed"
	dodoc README.txt *.flam3 || die "dodoc failed"
}

