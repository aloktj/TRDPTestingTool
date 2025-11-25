const fs = require('fs');
const xml2js = require('xml2js');

function toArray(value) {
  if (!value) {
    return [];
  }
  return Array.isArray(value) ? value : [value];
}

function toNumber(value) {
  if (value === undefined || value === null) {
    return undefined;
  }
  const parsed = Number(value);
  return Number.isNaN(parsed) ? undefined : parsed;
}

function deriveDirection(telegram) {
  const sources = toArray(telegram?.source);
  const destinations = toArray(telegram?.destination);
  const hasSource = sources.length > 0;
  const hasDestination = destinations.length > 0;

  if (hasSource && hasDestination) {
    return 'source+sink';
  }
  if (hasSource) {
    return 'source';
  }
  if (hasDestination) {
    return 'sink';
  }
  return 'unset';
}

class TrdpConfigLoader {
  constructor() {
    this.parser = new xml2js.Parser({ explicitArray: false, mergeAttrs: true });
  }

  async loadSummary(xmlPath) {
    const xmlContent = await fs.promises.readFile(xmlPath, 'utf8');
    const document = await this.parser.parseStringPromise(xmlContent);

    const device = document?.device || {};
    const deviceInfo = {
      hostName: device['host-name'] || device.hostName || '',
      type: device.type || '',
    };

    return {
      device: deviceInfo,
      interfaces: this.extractInterfaces(device),
      datasets: this.extractDatasets(device),
    };
  }

  extractInterfaces(device) {
    const busInterfaceList = device['bus-interface-list'];
    const interfaces = toArray(busInterfaceList?.['bus-interface']);

    return interfaces.map((iface) => {
      const telegrams = toArray(iface.telegram);
      const pdTelegrams = [];
      const mdTelegrams = [];

      telegrams.forEach((telegram) => {
        const pdParam = telegram['pd-parameter'];
        const mdParam = telegram['md-parameter'];
        const base = {
          name: telegram.name || '',
          comId: toNumber(telegram['com-id']),
          datasetId: toNumber(telegram['data-set-id']),
          cycle: undefined,
          direction: deriveDirection(telegram),
        };

        if (pdParam) {
          pdTelegrams.push({ ...base, cycle: toNumber(pdParam.cycle) });
        } else if (mdParam) {
          mdTelegrams.push({ ...base, cycle: toNumber(mdParam.cycle) });
        }
      });

      return {
        name: iface.name || '',
        networkId: toNumber(iface['network-id']),
        hostIp: iface['host-ip'] || '',
        pdTelegrams,
        mdTelegrams,
      };
    });
  }

  extractDatasets(device) {
    const datasetList = device['data-set-list'];
    const datasets = toArray(datasetList?.['data-set']);

    return datasets.map((dataset) => {
      const elements = toArray(dataset.element);
      return {
        id: toNumber(dataset.id),
        name: dataset.name || '',
        elementCount: elements.length,
      };
    });
  }
}

module.exports = TrdpConfigLoader;
