# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit eutils

DESCRIPTION="A cosmic recursive flame fractal editor written in Qt"
HOMEPAGE="http://qosmic.googlecode.com"
SRC_URI="${HOMEPAGE}/files/${P}.tar.bz2"
RESTRICT=mirror

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86 ~amd64"
IUSE=""

DEPEND=">=x11-libs/qt-4.3.1 \
	>=dev-libs/libxml2-2.6.28 \
	>=dev-lang/lua-5.1.1 \
	>=sci-mathematics/flam3-2.7.16 \
	>=media-libs/libpng-1.2.21-r3 \
	>=media-libs/jpeg-6b-r7"

S="${WORKDIR}/${PN}"

src_unpack() {
	unpack "${A}"
	cd "${S}"
	# Fix the install path
	sed -ie "s%PREFIX =.*%PREFIX = /usr%" ${S}/qosmic.pro \
		|| die 'sed failed while setting install path'
}

src_compile() {
	qmake || die "error while running qmake"
    emake || die "error while building qosmic"
}

src_install() {
	insinto /usr/share/pixmaps
	doins icons/qosmicicon.xpm
	dobin qosmic
	dodoc README
	dodoc README-LUA
	make_desktop_entry qosmic "Qosmic" qosmicicon.xpm "KDE;Qt;Graphics"
}
