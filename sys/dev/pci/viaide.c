/*	$NetBSD: viaide.c,v 1.57.4.3 2010/01/09 01:56:51 snj Exp $	*/

/*
 * Copyright (c) 1999, 2000, 2001 Manuel Bouyer.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Manuel Bouyer.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: viaide.c,v 1.57.4.3 2010/01/09 01:56:51 snj Exp $");

#include <sys/param.h>
#include <sys/systm.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>
#include <dev/pci/pciidereg.h>
#include <dev/pci/pciidevar.h>
#include <dev/pci/pciide_apollo_reg.h>

static int	via_pcib_match(struct pci_attach_args *);
static void	via_chip_map(struct pciide_softc *, struct pci_attach_args *);
static void	via_mapchan(struct pci_attach_args *, struct pciide_channel *,
		    pcireg_t, bus_size_t *, bus_size_t *, int (*)(void *));
static void	via_mapregs_compat_native(struct pci_attach_args *,
		    struct pciide_channel *, bus_size_t *, bus_size_t *);
static int	via_sata_chip_map_common(struct pciide_softc *,
		    struct pci_attach_args *);
static void	via_sata_chip_map(struct pciide_softc *,
		    struct pci_attach_args *, int);
static void	via_sata_chip_map_0(struct pciide_softc *,
		    struct pci_attach_args *);
static void	via_sata_chip_map_6(struct pciide_softc *,
		    struct pci_attach_args *);
static void	via_sata_chip_map_7(struct pciide_softc *,
		    struct pci_attach_args *);
static void	via_sata_chip_map_new(struct pciide_softc *,
		    struct pci_attach_args *);
static void	via_setup_channel(struct ata_channel *);

static int	viaide_match(device_t, cfdata_t, void *);
static void	viaide_attach(device_t, device_t, void *);
static const struct pciide_product_desc *
		viaide_lookup(pcireg_t);
static bool	viaide_suspend(device_t PMF_FN_PROTO);
static bool	viaide_resume(device_t PMF_FN_PROTO);

CFATTACH_DECL_NEW(viaide, sizeof(struct pciide_softc),
    viaide_match, viaide_attach, NULL, NULL);

static const struct pciide_product_desc pciide_amd_products[] =  {
	{ PCI_PRODUCT_AMD_PBC756_IDE,
	  0,
	  "Advanced Micro Devices AMD756 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_AMD_PBC766_IDE,
	  0,
	  "Advanced Micro Devices AMD766 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_AMD_PBC768_IDE,
	  0,
	  "Advanced Micro Devices AMD768 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_AMD_PBC8111_IDE,
	  0,
	  "Advanced Micro Devices AMD8111 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_AMD_CS5536_IDE,
	  0,
	  "Advanced Micro Devices CS5536 IDE Controller",
	  via_chip_map
	},
	{ 0,
	  0,
	  NULL,
	  NULL
	}
};

static const struct pciide_product_desc pciide_nvidia_products[] = {
	{ PCI_PRODUCT_NVIDIA_NFORCE_ATA100,
	  0,
	  "NVIDIA nForce IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_NVIDIA_NFORCE2_ATA133,
	  0,
	  "NVIDIA nForce2 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_NVIDIA_NFORCE2_400_ATA133,
	  0,
	  "NVIDIA nForce2 Ultra 400 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_NVIDIA_NFORCE2_400_SATA,
	  0,
	  "NVIDIA nForce2 Ultra 400 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_NFORCE3_ATA133,
	  0,
	  "NVIDIA nForce3 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_NVIDIA_NFORCE3_250_ATA133,
	  0,
	  "NVIDIA nForce3 250 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_NVIDIA_NFORCE3_250_SATA,
	  0,
	  "NVIDIA nForce3 250 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_NFORCE3_250_SATA2,
	  0,
	  "NVIDIA nForce3 250 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_NFORCE4_ATA133,
	  0,
	  "NVIDIA nForce4 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_NVIDIA_NFORCE4_SATA1,
	  0,
	  "NVIDIA nForce4 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_NFORCE4_SATA2,
	  0,
	  "NVIDIA nForce4 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_NFORCE430_ATA133,
	  0,
	  "NVIDIA nForce430 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_NVIDIA_NFORCE430_SATA1,
	  0,
	  "NVIDIA nForce430 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_NFORCE430_SATA2,
	  0,
	  "NVIDIA nForce430 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_MCP04_IDE,
	  0,
	  "NVIDIA MCP04 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_NVIDIA_MCP04_SATA,
	  0,
	  "NVIDIA MCP04 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_MCP04_SATA2,
	  0,
	  "NVIDIA MCP04 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_MCP55_IDE,
	  0,
	  "NVIDIA MCP55 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_NVIDIA_MCP55_SATA,
	  0,
	  "NVIDIA MCP55 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_MCP55_SATA2,
	  0,
	  "NVIDIA MCP55 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_MCP61_IDE,
	  0,
	  "NVIDIA MCP61 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_NVIDIA_MCP65_IDE,
	  0,
	  "NVIDIA MCP65 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_NVIDIA_MCP73_IDE,
	  0,
	  "NVIDIA MCP73 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_NVIDIA_MCP77_IDE,
	  0,
	  "NVIDIA MCP77 IDE Controller",
	  via_chip_map
	},
	{ PCI_PRODUCT_NVIDIA_MCP61_SATA,
	  0,
	  "NVIDIA MCP61 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_MCP61_SATA2,
	  0,
	  "NVIDIA MCP61 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_MCP61_SATA3,
	  0,
	  "NVIDIA MCP61 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_MCP65_SATA,
	  0,
	  "NVIDIA MCP65 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_MCP65_SATA2,
	  0,
	  "NVIDIA MCP65 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_MCP65_SATA3,
	  0,
	  "NVIDIA MCP65 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_MCP65_SATA4,
	  0,
	  "NVIDIA MCP65 Serial ATA Controller",
	  via_sata_chip_map_6
	},
	{ PCI_PRODUCT_NVIDIA_MCP67_IDE,
	  0,
	  "NVIDIA MCP67 IDE Controller",
	  via_chip_map,
	},
	{ PCI_PRODUCT_NVIDIA_MCP67_SATA,
	  0,
	  "NVIDIA MCP67 Serial ATA Controller",
	  via_sata_chip_map_6,
	},
	{ PCI_PRODUCT_NVIDIA_MCP67_SATA2,
	  0,
	  "NVIDIA MCP67 Serial ATA Controller",
	  via_sata_chip_map_6,
	},
	{ PCI_PRODUCT_NVIDIA_MCP67_SATA3,
	  0,
	  "NVIDIA MCP67 Serial ATA Controller",
	  via_sata_chip_map_6,
	},
	{ PCI_PRODUCT_NVIDIA_MCP67_SATA4,
	  0,
	  "NVIDIA MCP67 Serial ATA Controller",
	  via_sata_chip_map_6,
	},
	{ 0,
	  0,
	  NULL,
	  NULL
	}
};

static const struct pciide_product_desc pciide_via_products[] =  {
	{ PCI_PRODUCT_VIATECH_VT82C586_IDE,
	  0,
	  NULL,
	  via_chip_map,
	 },
	{ PCI_PRODUCT_VIATECH_VT82C586A_IDE,
	  0,
	  NULL,
	  via_chip_map,
	},
	{ PCI_PRODUCT_VIATECH_CX700_IDE,
	  0,
	  NULL,
	  via_chip_map,
	},
	{ PCI_PRODUCT_VIATECH_CX700M2_IDE,
	  0,
	  NULL,
	  via_chip_map,
	},
	{ PCI_PRODUCT_VIATECH_VT6421_RAID,
	  0,
	  "VIA Technologies VT6421 Serial RAID Controller",
	  via_sata_chip_map_new,
	},
	{ PCI_PRODUCT_VIATECH_VT8237_SATA,
	  0,
	  "VIA Technologies VT8237 SATA Controller",
	  via_sata_chip_map_7,
	},
	{ PCI_PRODUCT_VIATECH_VT8237A_SATA,
	  0,
	  "VIA Technologies VT8237A SATA Controller",
	  via_sata_chip_map_7,
	},
	{ PCI_PRODUCT_VIATECH_VT8237A_SATA_2,
	  0,
	  "VIA Technologies VT8237A (5337) SATA Controller",
	  via_sata_chip_map_7,
	},
	{ PCI_PRODUCT_VIATECH_VT8237R_SATA,
	  0,
	  "VIA Technologies VT8237R SATA Controller",
	  via_sata_chip_map_0,
	},
	{ 0,
	  0,
	  NULL,
	  NULL
	}
};

static const struct pciide_product_desc *
viaide_lookup(pcireg_t id)
{

	switch (PCI_VENDOR(id)) {
	case PCI_VENDOR_VIATECH:
		return (pciide_lookup_product(id, pciide_via_products));

	case PCI_VENDOR_AMD:
		return (pciide_lookup_product(id, pciide_amd_products));

	case PCI_VENDOR_NVIDIA:
		return (pciide_lookup_product(id, pciide_nvidia_products));
	}
	return (NULL);
}

static int
viaide_match(device_t parent, cfdata_t match, void *aux)
{
	struct pci_attach_args *pa = aux;

	if (viaide_lookup(pa->pa_id) != NULL)
		return (2);
	return (0);
}

static void
viaide_attach(device_t parent, device_t self, void *aux)
{
	struct pci_attach_args *pa = aux;
	struct pciide_softc *sc = device_private(self);
	const struct pciide_product_desc *pp;

	sc->sc_wdcdev.sc_atac.atac_dev = self;

	pp = viaide_lookup(pa->pa_id);
	if (pp == NULL)
		panic("viaide_attach");
	pciide_common_attach(sc, pa, pp);

	if (!pmf_device_register(self, viaide_suspend, viaide_resume))
		aprint_error_dev(self, "couldn't establish power handler\n");
}

static int
via_pcib_match(struct pci_attach_args *pa)
{
	if (PCI_CLASS(pa->pa_class) == PCI_CLASS_BRIDGE &&
	    PCI_SUBCLASS(pa->pa_class) == PCI_SUBCLASS_BRIDGE_ISA &&
	    PCI_VENDOR(pa->pa_id) == PCI_VENDOR_VIATECH)
		return (1);
	return 0;
}

static bool
viaide_suspend(device_t dv PMF_FN_ARGS)
{
	struct pciide_softc *sc = device_private(dv);

	sc->sc_pm_reg[0] = pci_conf_read(sc->sc_pc, sc->sc_tag, APO_IDECONF(sc));
	/* APO_DATATIM(sc) includes APO_UDMA(sc) */
	sc->sc_pm_reg[1] = pci_conf_read(sc->sc_pc, sc->sc_tag, APO_DATATIM(sc));
	/* This two are VIA-only, but should be ignored by other devices. */
	sc->sc_pm_reg[2] = pci_conf_read(sc->sc_pc, sc->sc_tag, APO_CTLMISC(sc));
	sc->sc_pm_reg[3] = pci_conf_read(sc->sc_pc, sc->sc_tag, APO_MISCTIM(sc));

	return true;
}

static bool
viaide_resume(device_t dv PMF_FN_ARGS)
{
	struct pciide_softc *sc = device_private(dv);

	pci_conf_write(sc->sc_pc, sc->sc_tag, APO_IDECONF(sc),
	    sc->sc_pm_reg[0]);
	pci_conf_write(sc->sc_pc, sc->sc_tag, APO_DATATIM(sc),
	    sc->sc_pm_reg[1]);
	/* This two are VIA-only, but should be ignored by other devices. */
	pci_conf_write(sc->sc_pc, sc->sc_tag, APO_CTLMISC(sc),
	    sc->sc_pm_reg[2]);
	pci_conf_write(sc->sc_pc, sc->sc_tag, APO_MISCTIM(sc),
	    sc->sc_pm_reg[3]);

	return true;
}

static void
via_chip_map(struct pciide_softc *sc, struct pci_attach_args *pa)
{
	struct pciide_channel *cp;
	pcireg_t interface = PCI_INTERFACE(pa->pa_class);
	pcireg_t vendor = PCI_VENDOR(pa->pa_id);
	int channel;
	u_int32_t ideconf;
	bus_size_t cmdsize, ctlsize;
	pcireg_t pcib_id, pcib_class;
	struct pci_attach_args pcib_pa;

	if (pciide_chipen(sc, pa) == 0)
		return;

	switch (vendor) {
	case PCI_VENDOR_VIATECH:
		/*
		 * get a PCI tag for the ISA bridge.
		 */
		if (pci_find_device(&pcib_pa, via_pcib_match) == 0)
			goto unknown;
		pcib_id = pcib_pa.pa_id;
		pcib_class = pcib_pa.pa_class;
		aprint_normal_dev(sc->sc_wdcdev.sc_atac.atac_dev,
		    "VIA Technologies ");
		switch (PCI_PRODUCT(pcib_id)) {
		case PCI_PRODUCT_VIATECH_VT82C586_ISA:
			aprint_normal("VT82C586 (Apollo VP) ");
			if(PCI_REVISION(pcib_class) >= 0x02) {
				aprint_normal("ATA33 controller\n");
				sc->sc_wdcdev.sc_atac.atac_udma_cap = 2;
			} else {
				aprint_normal("controller\n");
				sc->sc_wdcdev.sc_atac.atac_udma_cap = 0;
			}
			break;
		case PCI_PRODUCT_VIATECH_VT82C596A:
			aprint_normal("VT82C596A (Apollo Pro) ");
			if (PCI_REVISION(pcib_class) >= 0x12) {
				aprint_normal("ATA66 controller\n");
				sc->sc_wdcdev.sc_atac.atac_udma_cap = 4;
			} else {
				aprint_normal("ATA33 controller\n");
				sc->sc_wdcdev.sc_atac.atac_udma_cap = 2;
			}
			break;
		case PCI_PRODUCT_VIATECH_VT82C686A_ISA:
			aprint_normal("VT82C686A (Apollo KX133) ");
			if (PCI_REVISION(pcib_class) >= 0x40) {
				aprint_normal("ATA100 controller\n");
				sc->sc_wdcdev.sc_atac.atac_udma_cap = 5;
			} else {
				aprint_normal("ATA66 controller\n");
				sc->sc_wdcdev.sc_atac.atac_udma_cap = 4;
			}
			break;
		case PCI_PRODUCT_VIATECH_VT8231:
			aprint_normal("VT8231 ATA100 controller\n");
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 5;
			break;
		case PCI_PRODUCT_VIATECH_VT8233:
			aprint_normal("VT8233 ATA100 controller\n");
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 5;
			break;
		case PCI_PRODUCT_VIATECH_VT8233A:
			aprint_normal("VT8233A ATA133 controller\n");
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 6;
			break;
		case PCI_PRODUCT_VIATECH_VT8235:
			aprint_normal("VT8235 ATA133 controller\n");
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 6;
			break;
		case PCI_PRODUCT_VIATECH_VT8237:
			aprint_normal("VT8237 ATA133 controller\n");
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 6;
			break;
		case PCI_PRODUCT_VIATECH_VT8237A_ISA:
			aprint_normal("VT8237A ATA133 controller\n");
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 6;
			break;
		case PCI_PRODUCT_VIATECH_CX700_IDE:
			aprint_normal("CX700 ATA133 controller\n");
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 6;
			break;
		case PCI_PRODUCT_VIATECH_CX700M2_IDE:
			aprint_normal("CX700M2/VX700 ATA133 controller\n");
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 6;
			break;
		default:
unknown:
			aprint_normal("unknown VIA ATA controller\n");
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 0;
		}
		sc->sc_apo_regbase = APO_VIA_REGBASE;
		break;
	case PCI_VENDOR_AMD:
		switch (sc->sc_pp->ide_product) {
		case PCI_PRODUCT_AMD_PBC8111_IDE:
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 6;
			break;
		case PCI_PRODUCT_AMD_CS5536_IDE:
		case PCI_PRODUCT_AMD_PBC766_IDE:
		case PCI_PRODUCT_AMD_PBC768_IDE:
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 5;
			break;
		default:
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 4;
		}
		sc->sc_apo_regbase = APO_AMD_REGBASE;
		break;
	case PCI_VENDOR_NVIDIA:
		switch (sc->sc_pp->ide_product) {
		case PCI_PRODUCT_NVIDIA_NFORCE_ATA100:
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 5;
			break;
		case PCI_PRODUCT_NVIDIA_NFORCE2_ATA133:
		case PCI_PRODUCT_NVIDIA_NFORCE2_400_ATA133:
		case PCI_PRODUCT_NVIDIA_NFORCE3_ATA133:
		case PCI_PRODUCT_NVIDIA_NFORCE3_250_ATA133:
		case PCI_PRODUCT_NVIDIA_NFORCE4_ATA133:
		case PCI_PRODUCT_NVIDIA_NFORCE430_ATA133:
		case PCI_PRODUCT_NVIDIA_MCP04_IDE:
		case PCI_PRODUCT_NVIDIA_MCP55_IDE:
		case PCI_PRODUCT_NVIDIA_MCP61_IDE:
		case PCI_PRODUCT_NVIDIA_MCP65_IDE:
		case PCI_PRODUCT_NVIDIA_MCP67_IDE:
		case PCI_PRODUCT_NVIDIA_MCP73_IDE:
		case PCI_PRODUCT_NVIDIA_MCP77_IDE:
			sc->sc_wdcdev.sc_atac.atac_udma_cap = 6;
			break;
		}
		sc->sc_apo_regbase = APO_NVIDIA_REGBASE;
		break;
	default:
		panic("via_chip_map: unknown vendor");
	}

	aprint_verbose_dev(sc->sc_wdcdev.sc_atac.atac_dev,
	    "bus-master DMA support present");
	pciide_mapreg_dma(sc, pa);
	aprint_verbose("\n");
	sc->sc_wdcdev.sc_atac.atac_cap = ATAC_CAP_DATA16 | ATAC_CAP_DATA32;
	if (sc->sc_dma_ok) {
		sc->sc_wdcdev.sc_atac.atac_cap |= ATAC_CAP_DMA;
		sc->sc_wdcdev.irqack = pciide_irqack;
		if (sc->sc_wdcdev.sc_atac.atac_udma_cap > 0)
			sc->sc_wdcdev.sc_atac.atac_cap |= ATAC_CAP_UDMA;
	}
	sc->sc_wdcdev.sc_atac.atac_pio_cap = 4;
	sc->sc_wdcdev.sc_atac.atac_dma_cap = 2;
	sc->sc_wdcdev.sc_atac.atac_set_modes = via_setup_channel;
	sc->sc_wdcdev.sc_atac.atac_channels = sc->wdc_chanarray;
	sc->sc_wdcdev.sc_atac.atac_nchannels = PCIIDE_NUM_CHANNELS;

	if (PCI_CLASS(pa->pa_class) == PCI_CLASS_MASS_STORAGE &&
	    PCI_SUBCLASS(pa->pa_class) == PCI_SUBCLASS_MASS_STORAGE_RAID)
		sc->sc_wdcdev.sc_atac.atac_cap |= ATAC_CAP_RAID;

	wdc_allocate_regs(&sc->sc_wdcdev);

	ATADEBUG_PRINT(("via_chip_map: old APO_IDECONF=0x%x, "
	    "APO_CTLMISC=0x%x, APO_DATATIM=0x%x, APO_UDMA=0x%x\n",
	    pci_conf_read(sc->sc_pc, sc->sc_tag, APO_IDECONF(sc)),
	    pci_conf_read(sc->sc_pc, sc->sc_tag, APO_CTLMISC(sc)),
	    pci_conf_read(sc->sc_pc, sc->sc_tag, APO_DATATIM(sc)),
	    pci_conf_read(sc->sc_pc, sc->sc_tag, APO_UDMA(sc))),
	    DEBUG_PROBE);

	ideconf = pci_conf_read(sc->sc_pc, sc->sc_tag, APO_IDECONF(sc));
	for (channel = 0; channel < sc->sc_wdcdev.sc_atac.atac_nchannels;
	     channel++) {
		cp = &sc->pciide_channels[channel];
		if (pciide_chansetup(sc, channel, interface) == 0)
			continue;

		if ((ideconf & APO_IDECONF_EN(channel)) == 0) {
			aprint_normal_dev(sc->sc_wdcdev.sc_atac.atac_dev,
			    "%s channel ignored (disabled)\n", cp->name);
			cp->ata_channel.ch_flags |= ATACH_DISABLED;
			continue;
		}
		via_mapchan(pa, cp, interface, &cmdsize, &ctlsize,
		    pciide_pci_intr);
	}
}

static void
via_mapchan(struct pci_attach_args *pa,	struct pciide_channel *cp,
    pcireg_t interface, bus_size_t *cmdsizep, bus_size_t *ctlsizep,
    int (*pci_intr)(void *))
{
	struct ata_channel *wdc_cp;
	struct pciide_softc *sc;
	prop_bool_t compat_nat_enable;

	wdc_cp = &cp->ata_channel;
	sc = CHAN_TO_PCIIDE(&cp->ata_channel);
	compat_nat_enable = prop_dictionary_get(
	    device_properties(sc->sc_wdcdev.sc_atac.atac_dev),
	      "use-compat-native-irq");

	if (interface & PCIIDE_INTERFACE_PCI(wdc_cp->ch_channel)) {
		/* native mode with irq 14/15 requested? */
		if (compat_nat_enable != NULL &&
		    prop_bool_true(compat_nat_enable))
			via_mapregs_compat_native(pa, cp, cmdsizep, ctlsizep);
		else
			pciide_mapregs_native(pa, cp, cmdsizep, ctlsizep,
			    pci_intr);
	} else {
		pciide_mapregs_compat(pa, cp, wdc_cp->ch_channel, cmdsizep,
		    ctlsizep);
		if ((cp->ata_channel.ch_flags & ATACH_DISABLED) == 0)
			pciide_map_compat_intr(pa, cp, wdc_cp->ch_channel);
	}
	wdcattach(wdc_cp);
}

/*
 * At least under certain (mis)configurations (e.g. on the "Pegasos" board)
 * the VT8231-IDE's native mode only works with irq 14/15, and cannot be
 * programmed to use a single native PCI irq alone. So we install an interrupt
 * handler for each channel, as in compatibility mode.
 */
static void
via_mapregs_compat_native(struct pci_attach_args *pa,
    struct pciide_channel *cp, bus_size_t *cmdsizep, bus_size_t *ctlsizep)
{
	struct ata_channel *wdc_cp;
	struct pciide_softc *sc;

	wdc_cp = &cp->ata_channel;
	sc = CHAN_TO_PCIIDE(&cp->ata_channel);

	/* XXX prevent pciide_mapregs_native from installing a handler */
	if (sc->sc_pci_ih == NULL)
		sc->sc_pci_ih = (void *)~0;
	pciide_mapregs_native(pa, cp, cmdsizep, ctlsizep, NULL);

	/* interrupts are fixed to 14/15, as in compatibility mode */
	cp->compat = 1;
	if ((wdc_cp->ch_flags & ATACH_DISABLED) == 0) {
#ifdef __HAVE_PCIIDE_MACHDEP_COMPAT_INTR_ESTABLISH
		cp->ih = pciide_machdep_compat_intr_establish(
		    sc->sc_wdcdev.sc_atac.atac_dev, pa, wdc_cp->ch_channel,
		    pciide_compat_intr, cp);
		if (cp->ih == NULL) {
#endif
			aprint_error_dev(sc->sc_wdcdev.sc_atac.atac_dev,
			    "no compatibility interrupt for "
			    "use by %s channel\n", cp->name);
			wdc_cp->ch_flags |= ATACH_DISABLED;
#ifdef __HAVE_PCIIDE_MACHDEP_COMPAT_INTR_ESTABLISH
		}
		sc->sc_pci_ih = cp->ih;  /* XXX */
#endif
	}
}

static void
via_setup_channel(struct ata_channel *chp)
{
	u_int32_t udmatim_reg, datatim_reg;
	u_int8_t idedma_ctl;
	int mode, drive, s;
	struct ata_drive_datas *drvp;
	struct atac_softc *atac = chp->ch_atac;
	struct pciide_channel *cp = CHAN_TO_PCHAN(chp);
	struct pciide_softc *sc = CHAN_TO_PCIIDE(chp);
#ifndef PCIIDE_AMD756_ENABLEDMA
	int rev = PCI_REVISION(
	    pci_conf_read(sc->sc_pc, sc->sc_tag, PCI_CLASS_REG));
#endif

	idedma_ctl = 0;
	datatim_reg = pci_conf_read(sc->sc_pc, sc->sc_tag, APO_DATATIM(sc));
	udmatim_reg = pci_conf_read(sc->sc_pc, sc->sc_tag, APO_UDMA(sc));
	datatim_reg &= ~APO_DATATIM_MASK(chp->ch_channel);
	udmatim_reg &= ~APO_UDMA_MASK(chp->ch_channel);

	/* setup DMA if needed */
	pciide_channel_dma_setup(cp);

	for (drive = 0; drive < 2; drive++) {
		drvp = &chp->ch_drive[drive];
		/* If no drive, skip */
		if ((drvp->drive_flags & DRIVE) == 0)
			continue;
		/* add timing values, setup DMA if needed */
		if (((drvp->drive_flags & DRIVE_DMA) == 0 &&
		    (drvp->drive_flags & DRIVE_UDMA) == 0)) {
			mode = drvp->PIO_mode;
			goto pio;
		}
		if ((atac->atac_cap & ATAC_CAP_UDMA) &&
		    (drvp->drive_flags & DRIVE_UDMA)) {
			/* use Ultra/DMA */
			s = splbio();
			drvp->drive_flags &= ~DRIVE_DMA;
			splx(s);
			udmatim_reg |= APO_UDMA_EN(chp->ch_channel, drive) |
			    APO_UDMA_EN_MTH(chp->ch_channel, drive);
			switch (PCI_VENDOR(sc->sc_pci_id)) {
			case PCI_VENDOR_VIATECH:
				if (sc->sc_wdcdev.sc_atac.atac_udma_cap == 6) {
					/* 8233a */
					udmatim_reg |= APO_UDMA_TIME(
					    chp->ch_channel,
					    drive,
					    via_udma133_tim[drvp->UDMA_mode]);
				} else if (sc->sc_wdcdev.sc_atac.atac_udma_cap == 5) {
					/* 686b */
					udmatim_reg |= APO_UDMA_TIME(
					    chp->ch_channel,
					    drive,
					    via_udma100_tim[drvp->UDMA_mode]);
				} else if (sc->sc_wdcdev.sc_atac.atac_udma_cap == 4) {
					/* 596b or 686a */
					udmatim_reg |= APO_UDMA_CLK66(
					    chp->ch_channel);
					udmatim_reg |= APO_UDMA_TIME(
					    chp->ch_channel,
					    drive,
					    via_udma66_tim[drvp->UDMA_mode]);
				} else {
					/* 596a or 586b */
					udmatim_reg |= APO_UDMA_TIME(
					    chp->ch_channel,
					    drive,
					    via_udma33_tim[drvp->UDMA_mode]);
				}
				break;
			case PCI_VENDOR_AMD:
			case PCI_VENDOR_NVIDIA:
				udmatim_reg |= APO_UDMA_TIME(chp->ch_channel,
				    drive, amd7x6_udma_tim[drvp->UDMA_mode]);
				 break;
			}
			/* can use PIO timings, MW DMA unused */
			mode = drvp->PIO_mode;
		} else {
			/* use Multiword DMA, but only if revision is OK */
			s = splbio();
			drvp->drive_flags &= ~DRIVE_UDMA;
			splx(s);
#ifndef PCIIDE_AMD756_ENABLEDMA
			/*
			 * The workaround doesn't seem to be necessary
			 * with all drives, so it can be disabled by
			 * PCIIDE_AMD756_ENABLEDMA. It causes a hard hang if
			 * triggered.
			 */
			if (PCI_VENDOR(sc->sc_pci_id) == PCI_VENDOR_AMD &&
			    sc->sc_pp->ide_product ==
			    PCI_PRODUCT_AMD_PBC756_IDE &&
			    AMD756_CHIPREV_DISABLEDMA(rev)) {
				aprint_normal(
				    "%s:%d:%d: multi-word DMA disabled due "
				    "to chip revision\n",
				    device_xname(
				      sc->sc_wdcdev.sc_atac.atac_dev),
				    chp->ch_channel, drive);
				mode = drvp->PIO_mode;
				s = splbio();
				drvp->drive_flags &= ~DRIVE_DMA;
				splx(s);
				goto pio;
			}
#endif
			/* mode = min(pio, dma+2) */
			if (drvp->PIO_mode <= (drvp->DMA_mode + 2))
				mode = drvp->PIO_mode;
			else
				mode = drvp->DMA_mode + 2;
		}
		idedma_ctl |= IDEDMA_CTL_DRV_DMA(drive);

pio:		/* setup PIO mode */
		if (mode <= 2) {
			drvp->DMA_mode = 0;
			drvp->PIO_mode = 0;
			mode = 0;
		} else {
			drvp->PIO_mode = mode;
			drvp->DMA_mode = mode - 2;
		}
		datatim_reg |=
		    APO_DATATIM_PULSE(chp->ch_channel, drive,
			apollo_pio_set[mode]) |
		    APO_DATATIM_RECOV(chp->ch_channel, drive,
			apollo_pio_rec[mode]);
	}
	if (idedma_ctl != 0) {
		/* Add software bits in status register */
		bus_space_write_1(sc->sc_dma_iot, cp->dma_iohs[IDEDMA_CTL], 0,
		    idedma_ctl);
	}
	pci_conf_write(sc->sc_pc, sc->sc_tag, APO_DATATIM(sc), datatim_reg);
	pci_conf_write(sc->sc_pc, sc->sc_tag, APO_UDMA(sc), udmatim_reg);
	ATADEBUG_PRINT(("via_chip_map: APO_DATATIM=0x%x, APO_UDMA=0x%x\n",
	    pci_conf_read(sc->sc_pc, sc->sc_tag, APO_DATATIM(sc)),
	    pci_conf_read(sc->sc_pc, sc->sc_tag, APO_UDMA(sc))), DEBUG_PROBE);
}

static int
via_sata_chip_map_common(struct pciide_softc *sc, struct pci_attach_args *pa)
{
	bus_size_t satasize;
	int maptype, ret;

	if (pciide_chipen(sc, pa) == 0)
		return 0;

	aprint_verbose_dev(sc->sc_wdcdev.sc_atac.atac_dev,
	    "bus-master DMA support present");
	pciide_mapreg_dma(sc, pa);
	aprint_verbose("\n");

	/*
	 * Enable memory-space access if it isn't already there.
	 */
	if (pa->pa_memt && (pa->pa_flags & PCI_FLAGS_MEM_ENABLED) == 0) {
		pcireg_t csr;

		pa->pa_flags |= PCI_FLAGS_MEM_ENABLED;
		csr = pci_conf_read(pa->pa_pc, pa->pa_tag, PCI_COMMAND_STATUS_REG);
		pci_conf_write(pa->pa_pc, pa->pa_tag, PCI_COMMAND_STATUS_REG,
		               csr | PCI_COMMAND_MEM_ENABLE);
	}

	if (sc->sc_dma_ok) {
		sc->sc_wdcdev.sc_atac.atac_cap |= ATAC_CAP_UDMA | ATAC_CAP_DMA;
		sc->sc_wdcdev.irqack = pciide_irqack;
	}
	sc->sc_wdcdev.sc_atac.atac_pio_cap = 4;
	sc->sc_wdcdev.sc_atac.atac_dma_cap = 2;
	sc->sc_wdcdev.sc_atac.atac_udma_cap = 6;

	sc->sc_wdcdev.sc_atac.atac_channels = sc->wdc_chanarray;
	sc->sc_wdcdev.sc_atac.atac_nchannels = PCIIDE_NUM_CHANNELS;
	sc->sc_wdcdev.sc_atac.atac_cap |= ATAC_CAP_DATA16 | ATAC_CAP_DATA32;
	sc->sc_wdcdev.sc_atac.atac_set_modes = sata_setup_channel;

	if (PCI_CLASS(pa->pa_class) == PCI_CLASS_MASS_STORAGE &&
	    PCI_SUBCLASS(pa->pa_class) == PCI_SUBCLASS_MASS_STORAGE_RAID)
		sc->sc_wdcdev.sc_atac.atac_cap |= ATAC_CAP_RAID;

	wdc_allocate_regs(&sc->sc_wdcdev);
	maptype = pci_mapreg_type(pa->pa_pc, pa->pa_tag,
	    PCI_MAPREG_START + 0x14);
	switch(maptype) {
	case PCI_MAPREG_TYPE_IO:
		ret = pci_mapreg_map(pa, PCI_MAPREG_START + 0x14,
		    PCI_MAPREG_TYPE_IO, 0, &sc->sc_ba5_st, &sc->sc_ba5_sh,
		    NULL, &satasize);
		break;
	case PCI_MAPREG_MEM_TYPE_32BIT:
		ret = pci_mapreg_map(pa, PCI_MAPREG_START + 0x14,
		    PCI_MAPREG_TYPE_MEM | PCI_MAPREG_MEM_TYPE_32BIT,
		    0, &sc->sc_ba5_st, &sc->sc_ba5_sh,
		    NULL, &satasize);
		break;
	default:
		aprint_error_dev(sc->sc_wdcdev.sc_atac.atac_dev,
		    "couldn't map sata regs, unsupported maptype (0x%x)\n",
		    maptype);
		return 0;
	}
	if (ret != 0) {
		aprint_error_dev(sc->sc_wdcdev.sc_atac.atac_dev,
		    "couldn't map sata regs\n");
		return 0;
	}
	return 1;
}

static void
via_sata_chip_map(struct pciide_softc *sc, struct pci_attach_args *pa,
    int satareg_shift)
{
	struct pciide_channel *cp;
	struct ata_channel *wdc_cp;
	struct wdc_regs *wdr;
	pcireg_t interface = PCI_INTERFACE(pa->pa_class);
	int channel;
	bus_size_t cmdsize, ctlsize;

	if (via_sata_chip_map_common(sc, pa) == 0)
		return;

	if (interface == 0) {
		ATADEBUG_PRINT(("via_sata_chip_map interface == 0\n"),
		    DEBUG_PROBE);
		interface = PCIIDE_INTERFACE_BUS_MASTER_DMA |
		    PCIIDE_INTERFACE_PCI(0) | PCIIDE_INTERFACE_PCI(1);
	}

	for (channel = 0; channel < sc->sc_wdcdev.sc_atac.atac_nchannels;
	     channel++) {
		cp = &sc->pciide_channels[channel];
		if (pciide_chansetup(sc, channel, interface) == 0)
			continue;
		wdc_cp = &cp->ata_channel;
		wdr = CHAN_TO_WDC_REGS(wdc_cp);
		wdr->sata_iot = sc->sc_ba5_st;
		wdr->sata_baseioh = sc->sc_ba5_sh;
		if (bus_space_subregion(wdr->sata_iot, wdr->sata_baseioh,
		    (wdc_cp->ch_channel << satareg_shift) + 0x0, 1,
		    &wdr->sata_status) != 0) {
			aprint_error_dev(sc->sc_wdcdev.sc_atac.atac_dev,
			    "couldn't map channel %d sata_status regs\n",
			    wdc_cp->ch_channel);
			continue;
		}
		if (bus_space_subregion(wdr->sata_iot, wdr->sata_baseioh,
		    (wdc_cp->ch_channel << satareg_shift) + 0x4, 1,
		    &wdr->sata_error) != 0) {
			aprint_error_dev(sc->sc_wdcdev.sc_atac.atac_dev,
			    "couldn't map channel %d sata_error regs\n",
			    wdc_cp->ch_channel);
			continue;
		}
		if (bus_space_subregion(wdr->sata_iot, wdr->sata_baseioh,
		    (wdc_cp->ch_channel << satareg_shift) + 0x8, 1,
		    &wdr->sata_control) != 0) {
			aprint_error_dev(sc->sc_wdcdev.sc_atac.atac_dev,
			    "couldn't map channel %d sata_control regs\n",
			    wdc_cp->ch_channel);
			continue;
		}
		sc->sc_wdcdev.sc_atac.atac_probe = wdc_sataprobe;
		pciide_mapchan(pa, cp, interface, &cmdsize, &ctlsize,
		    pciide_pci_intr);
	}
}

static void
via_sata_chip_map_0(struct pciide_softc *sc, struct pci_attach_args *pa)
{
	via_sata_chip_map(sc, pa, 0);
}

static void
via_sata_chip_map_6(struct pciide_softc *sc, struct pci_attach_args *pa)
{
	via_sata_chip_map(sc, pa, 6);
}

static void
via_sata_chip_map_7(struct pciide_softc *sc, struct pci_attach_args *pa)
{
	via_sata_chip_map(sc, pa, 7);
}

static void
via_sata_chip_map_new(struct pciide_softc *sc, struct pci_attach_args *pa)
{
	struct pciide_channel *cp;
	struct ata_channel *wdc_cp;
	struct wdc_regs *wdr;
	pcireg_t interface = PCI_INTERFACE(pa->pa_class);
	int channel;
	bus_size_t cmdsize;
	pci_intr_handle_t intrhandle;
	const char *intrstr;
	int i;

	if (via_sata_chip_map_common(sc, pa) == 0)
		return;

	if (interface == 0) {
		ATADEBUG_PRINT(("via_sata_chip_map interface == 0\n"),
		    DEBUG_PROBE);
		interface = PCIIDE_INTERFACE_BUS_MASTER_DMA |
		    PCIIDE_INTERFACE_PCI(0) | PCIIDE_INTERFACE_PCI(1);
	}

	if (pci_intr_map(pa, &intrhandle) != 0) {
		aprint_error_dev(sc->sc_wdcdev.sc_atac.atac_dev,
		    "couldn't map native-PCI interrupt\n");
		return;
	}
	intrstr = pci_intr_string(pa->pa_pc, intrhandle);
	sc->sc_pci_ih = pci_intr_establish(pa->pa_pc,
	    intrhandle, IPL_BIO, pciide_pci_intr, sc);
	if (sc->sc_pci_ih == NULL) {
		aprint_error_dev(sc->sc_wdcdev.sc_atac.atac_dev,
		    "couldn't establish native-PCI interrupt");
		if (intrstr != NULL)
		    aprint_error(" at %s", intrstr);
		aprint_error("\n");
		return;
	}
	aprint_normal_dev(sc->sc_wdcdev.sc_atac.atac_dev,
	    "using %s for native-PCI interrupt\n",
	    intrstr ? intrstr : "unknown interrupt");

	for (channel = 0; channel < sc->sc_wdcdev.sc_atac.atac_nchannels;
	     channel++) {
		cp = &sc->pciide_channels[channel];
		if (pciide_chansetup(sc, channel, interface) == 0)
			continue;
		cp->ata_channel.ch_ndrive = 1;
		wdc_cp = &cp->ata_channel;
		wdr = CHAN_TO_WDC_REGS(wdc_cp);

		wdr->sata_iot = sc->sc_ba5_st;
		wdr->sata_baseioh = sc->sc_ba5_sh;
		if (bus_space_subregion(wdr->sata_iot, wdr->sata_baseioh,
		    (wdc_cp->ch_channel << 6) + 0x0, 1,
		    &wdr->sata_status) != 0) {
			aprint_error_dev(sc->sc_wdcdev.sc_atac.atac_dev,
			    "couldn't map channel %d sata_status regs\n",
			    wdc_cp->ch_channel);
			continue;
		}
		if (bus_space_subregion(wdr->sata_iot, wdr->sata_baseioh,
		    (wdc_cp->ch_channel << 6) + 0x4, 1,
		    &wdr->sata_error) != 0) {
			aprint_error_dev(sc->sc_wdcdev.sc_atac.atac_dev,
			    "couldn't map channel %d sata_error regs\n",
			    wdc_cp->ch_channel);
			continue;
		}
		if (bus_space_subregion(wdr->sata_iot, wdr->sata_baseioh,
		    (wdc_cp->ch_channel << 6) + 0x8, 1,
		    &wdr->sata_control) != 0) {
			aprint_error_dev(sc->sc_wdcdev.sc_atac.atac_dev,
			    "couldn't map channel %d sata_control regs\n",
			    wdc_cp->ch_channel);
			continue;
		}
		sc->sc_wdcdev.sc_atac.atac_probe = wdc_sataprobe;

		if (pci_mapreg_map(pa, (0x10 + (4 * (channel))),
		    PCI_MAPREG_TYPE_IO, 0, &wdr->cmd_iot, &wdr->cmd_baseioh,
		    NULL, &cmdsize) != 0) {
			aprint_error_dev(sc->sc_wdcdev.sc_atac.atac_dev,
			    "couldn't map %s channel regs\n", cp->name);
		}
		wdr->ctl_iot = wdr->cmd_iot;
		for (i = 0; i < WDC_NREG; i++) {
			if (bus_space_subregion(wdr->cmd_iot,
			    wdr->cmd_baseioh, i, i == 0 ? 4 : 1,
			    &wdr->cmd_iohs[i]) != 0) {
				aprint_error_dev(
				    sc->sc_wdcdev.sc_atac.atac_dev,
				    "couldn't subregion %s "
				    "channel cmd regs\n", cp->name);
				return;
			}
		}
		if (bus_space_subregion(wdr->cmd_iot, wdr->cmd_baseioh,
		    WDC_NREG + 2, 1,  &wdr->ctl_ioh) != 0) {
			aprint_error_dev(sc->sc_wdcdev.sc_atac.atac_dev,
			    "couldn't map channel %d ctl regs\n", channel);
			return;
		}
		wdc_init_shadow_regs(wdc_cp);
		wdr->data32iot = wdr->cmd_iot;
		wdr->data32ioh = wdr->cmd_iohs[wd_data];
		wdcattach(wdc_cp);
	}
}
