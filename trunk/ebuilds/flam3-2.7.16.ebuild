# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit eutils

DESCRIPTION="Tools and a library for creating flame fractal images"
HOMEPAGE="http://flam3.com/"
SRC_URI="${HOMEPAGE}/${P}.tar.gz"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="~x86 ~amd64"
IUSE=""

DEPEND=">=dev-libs/libxml2-2.6.28 \
	>=media-libs/libpng-1.2.21-r3 \
	>=media-libs/jpeg-6b-r7"


#src_unpack() {
#    unpack ${A}
#    cd "${S}"
#    epatch "${FILESDIR}"/${PN}-2.7.9-verbosity.patch
#}

src_compile() {
#    econf --includedir=/usr/include/flam3 || die "econf failed"
	econf || die "econf failed"
	emake || die "emake compile failed"
}

src_install() {
	emake install DESTDIR="${D}" || die "emake install failed"
 	dodoc README.txt
 	dodoc src/genomes/*.flam3
}
