# Copyright 1999-2016 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Id$

EAPI=4

inherit qmake-utils

DESCRIPTION="A cosmic recursive flame fractal editor"
HOMEPAGE="https://github.com/bitsed/qosmic"
SRC_URI="https://github.com/bitsed/qosmic/archive/v${PV}.tar.gz -> ${P}.tar.gz"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS="~amd64"
IUSE=""

RDEPEND="dev-lang/lua:5.2
	dev-qt/qtgui:5
	>=media-gfx/flam3-3.1.1"
DEPEND="${RDEPEND}
	virtual/pkgconfig"

DOCS="changes.txt README README-LUA"

src_prepare() {
	eqmake5 ROOT=${D}
}
