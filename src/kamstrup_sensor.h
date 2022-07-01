#include "esphome.h"
#include "src/mbusparser.h"
#include "src/gcm.h"
#include "src/secrets.h"

const size_t headersize = 11;
const size_t footersize = 3;
uint8_t encryption_key[16];
uint8_t authentication_key[16];
uint8_t receiveBuffer[500];
uint8_t decryptedFrameBuffer[500];
VectorView decryptedFrame(decryptedFrameBuffer, 0);
MbusStreamParser streamParser(receiveBuffer, sizeof(receiveBuffer));
mbedtls_gcm_context m_ctx;

class KamstrupComponent : public Component, public UARTDevice {
    public:
         Sensor *activePowerPlusValid = new Sensor();
         Sensor *activePowerMinusValid = new Sensor();
         Sensor *activePowerPlusValidL1 = new Sensor();
         Sensor *activePowerMinusValidL1 = new Sensor();
         Sensor *activePowerPlusValidL2 = new Sensor();
         Sensor *activePowerMinusValidL2 = new Sensor();
         Sensor *activePowerPlusValidL3 = new Sensor();
         Sensor *activePowerMinusValidL3 = new Sensor();
         Sensor *reactivePowerPlusValid = new Sensor();
         Sensor *reactivePowerMinusValid = new Sensor();

         Sensor *powerFactorValidL1 = new Sensor();
         Sensor *powerFactorValidL2 = new Sensor();
         Sensor *powerFactorValidL3 = new Sensor();
         Sensor *powerFactorTotalValid = new Sensor();

         Sensor *voltageL1Valid = new Sensor();
         Sensor *voltageL2Valid = new Sensor();
         Sensor *voltageL3Valid = new Sensor();

         Sensor *centiAmpereL1Valid = new Sensor();
         Sensor *centiAmpereL2Valid = new Sensor();
         Sensor *centiAmpereL3Valid = new Sensor();

         Sensor *energyActiveImportWh = new Sensor();
         Sensor *activeExportWhValid = new Sensor();
         Sensor *activeImportWhValidL1 = new Sensor();
         Sensor *activeExportWhValidL1 = new Sensor();
         Sensor *activeImportWhValidL2 = new Sensor();
         Sensor *activeExportWhValidL2 = new Sensor();
         Sensor *activeImportWhValidL3 = new Sensor();
         Sensor *activeExportWhValidL3 = new Sensor();

         Sensor *reactiveImportWhValid = new Sensor();
         Sensor *reactiveExportWhValid = new Sensor();


		KamstrupComponent(UARTComponent *parent) : Component(),  UARTDevice(parent) { }

		void setup() override {
          ESP_LOGI("kamstrup", "Initializing device and encryption keys");
          hexStr2bArr(encryption_key, conf_key, sizeof(encryption_key));
          hexStr2bArr(authentication_key, conf_authkey, sizeof(authentication_key));
          ESP_LOGI("kamstrup", "Initialization completed");
       }

       void loop() override {
           if (Serial.available() <= 0) { return; }
           while (Serial.available() > 0) {
             if (streamParser.pushData(Serial.read()) == false) { return; }
             VectorView frame = streamParser.getFrame();
             if (streamParser.getContentType() != MbusStreamParser::COMPLETE_FRAME) { return; }
               ESP_LOGI("kamstrup", "Frame complete");
               if (!decrypt(frame)) { ESP_LOGW("kamstrup", "Decryption failed"); return; }
               MeterData md = parseMbusFrame(decryptedFrame);
               sendData(md);
           }
       }

       void sendData(MeterData md) {
         if (md.activePowerPlusValid) { activePowerPlusValid->publish_state(md.activePowerPlusValid); }
         if (md.activePowerMinusValid) { activePowerMinusValid->publish_state(md.activePowerMinusValid); }
         if (md.activePowerPlusValidL1) { activePowerPlusValidL1->publish_state(md.activePowerPlusValidL1); }
         if (md.activePowerMinusValidL1) { activePowerMinusValidL1->publish_state(md.activePowerMinusValidL1); }
         if (md.activePowerPlusValidL2) { activePowerPlusValidL2->publish_state(md.activePowerPlusValidL2); }
         if (md.activePowerMinusValidL2) { activePowerMinusValidL2->publish_state(md.activePowerMinusValidL2); }
         if (md.activePowerPlusValidL3) { activePowerPlusValidL3->publish_state(md.activePowerPlusValidL3); }
         if (md.activePowerMinusValidL3) { activePowerMinusValidL3->publish_state(md.activePowerMinusValidL3); }
         if (md.reactivePowerPlusValid) { reactivePowerPlusValid->publish_state(md.reactivePowerPlusValid); }
         if (md.reactivePowerMinusValid) { reactivePowerMinusValid->publish_state(md.reactivePowerMinusValid); }

         if (md.powerFactorValidL1) { powerFactorValidL1->publish_state(md.powerFactorValidL1); }
         if (md.powerFactorValidL2) { powerFactorValidL2->publish_state(md.powerFactorValidL2); }
         if (md.powerFactorValidL3) { powerFactorValidL3->publish_state(md.powerFactorValidL3); }
         if (md.powerFactorTotalValid) { powerFactorTotalValid->publish_state(md.powerFactorTotalValid); }

         if (md.voltageL1Valid) { voltageL1Valid->publish_state(md.voltageL1Valid); }
         if (md.voltageL2Valid) { voltageL2Valid->publish_state(md.voltageL2Valid); }
         if (md.voltageL3Valid) { voltageL3Valid->publish_state(md.voltageL3Valid); }

         if (md.centiAmpereL1Valid) { centiAmpereL1Valid->publish_state(md.centiAmpereL1Valid); }
         if (md.centiAmpereL2Valid) { centiAmpereL2Valid->publish_state(md.centiAmpereL2Valid); }
         if (md.centiAmpereL3Valid) { centiAmpereL3Valid->publish_state(md.centiAmpereL3Valid); }

         if (md.activeImportWhValid) { energyActiveImportWh->publish_state(md.activeImportWh); }
         if (md.activeExportWhValid) { activeExportWhValid->publish_state(md.activeExportWhValid); }
         if (md.activeImportWhValidL1) { activeImportWhValidL1->publish_state(md.activeImportWhValidL1); }
         if (md.activeExportWhValidL1) { activeExportWhValidL1->publish_state(md.activeExportWhValidL1); }
         if (md.activeImportWhValidL2) { activeImportWhValidL2->publish_state(md.activeImportWhValidL2); }
         if (md.activeExportWhValidL2) { activeExportWhValidL2->publish_state(md.activeExportWhValidL2); }
         if (md.activeImportWhValidL3) { activeImportWhValidL3->publish_state(md.activeImportWhValidL3); }
         if (md.activeExportWhValidL3) { activeExportWhValidL3->publish_state(md.activeExportWhValidL3); }

         if (md.reactiveImportWhValid) { reactiveImportWhValid->publish_state(md.reactiveImportWhValid); }
         if (md.reactiveExportWhValid) { reactiveExportWhValid->publish_state(md.reactiveExportWhValid); }
       }

       void printHex(const VectorView& frame) {
           for (int i = 0; i < frame.size(); i++) {
               Serial.printf("%02X", frame[i]);
           }
       }

       bool decrypt(const VectorView& frame) {

           if (frame.size() < headersize + footersize + 12 + 18) {
             ESP_LOGW("kamstrup", "Invalid frame size.");
           }

           memcpy(decryptedFrameBuffer, &frame.front(), frame.size());

           uint8_t system_title[8];
           memcpy(system_title, decryptedFrameBuffer + headersize + 2, 8);

           uint8_t initialization_vector[12];
           memcpy(initialization_vector, system_title, 8);
           memcpy(initialization_vector + 8, decryptedFrameBuffer + headersize + 14, 4);

           uint8_t additional_authenticated_data[17];
           memcpy(additional_authenticated_data, decryptedFrameBuffer + headersize + 13, 1);
           memcpy(additional_authenticated_data + 1, authentication_key, 16);

           uint8_t authentication_tag[12];
           memcpy(authentication_tag, decryptedFrameBuffer + headersize + frame.size() - headersize - footersize - 12, 12);

           uint8_t cipher_text[frame.size() - headersize - footersize - 18 - 12];
           memcpy(cipher_text, decryptedFrameBuffer + headersize + 18, frame.size() - headersize - footersize - 12 - 18);

           uint8_t plaintext[sizeof(cipher_text)];

           mbedtls_gcm_init(&m_ctx);
           int success = mbedtls_gcm_setkey(&m_ctx, MBEDTLS_CIPHER_ID_AES, encryption_key, sizeof(encryption_key) * 8);
           if (0 != success) {
             ESP_LOGE("kamstrup", "Setkey failed: ");
             return false;
           }
           success = mbedtls_gcm_auth_decrypt(&m_ctx, sizeof(cipher_text), initialization_vector, sizeof(initialization_vector),
                   additional_authenticated_data, sizeof(additional_authenticated_data), authentication_tag, sizeof(authentication_tag),
                   cipher_text, plaintext);
           if (0 != success) {
             ESP_LOGE("kamstrup", "authdecrypt failed: ");
				return false;
			}
			mbedtls_gcm_free(&m_ctx);

			//copy replace encrypted data with decrypted for mbusparser library. Checksum not updated. Hopefully not needed
			memcpy(decryptedFrameBuffer + headersize + 18, plaintext, sizeof(plaintext));
			decryptedFrame = VectorView(decryptedFrameBuffer, frame.size());

            ESP_LOGI("kamstrup", "payload decrypted successfully");
			return true;
		}

		void hexStr2bArr(uint8_t* dest, const char* source, int bytes_n)
		{
			uint8_t* dst = dest;
			uint8_t* end = dest + sizeof(bytes_n);
			unsigned int u;

			while (dest < end && sscanf(source, "%2x", &u) == 1)
			{
				*dst++ = u;
				source += 2;
			}
		}

  float get_setup_priority() const override { return esphome::setup_priority::LATE; }
};
