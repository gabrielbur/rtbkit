/* rtbkit_exchange_connector.cc
   Mathieu Stefani, 15 May 2013
   
   Implementation of the RTBKit exchange connector.
*/

#include "rtbkit_exchange_connector.h"
#include "rtbkit/plugins/exchange/http_auction_handler.h"

using namespace Datacratic;

namespace RTBKIT {

/*****************************************************************************/
/* RTBKIT EXCHANGE CONNECTOR                                                 */
/*****************************************************************************/

RTBKitExchangeConnector::
RTBKitExchangeConnector(ServiceBase &owner, const std::string &name)
    : OpenRTBExchangeConnector(owner, name)
{
}

RTBKitExchangeConnector::
RTBKitExchangeConnector(const std::string &name,
                        std::shared_ptr<ServiceProxies> proxies)
    : OpenRTBExchangeConnector(name, proxies)
{
}

std::shared_ptr<BidRequest>
RTBKitExchangeConnector::
parseBidRequest(HttpAuctionHandler &connection,
                const HttpHeader &header,
                const std::string &payload)
{
    auto request = 
        OpenRTBExchangeConnector::parseBidRequest(connection, header, payload);


    if (request != nullptr) {
        for (const auto &imp: request->imp) {
            if (!imp.ext.isMember("external-ids")) {
                connection.sendErrorResponse("MISSING_EXTENSION_FIELD",
                    ML::format("The impression '%s' requires the 'external-ids' extension field",
                               imp.id.toString()));  
                request.reset();
                break;
            }
        }
    }

    return request;
}

void
RTBKitExchangeConnector::
setSeatBid(const Auction & auction,
           int spotNum,
           OpenRTB::BidResponse &response) const
{
    // Same as OpenRTB
    OpenRTBExchangeConnector::setSeatBid(auction, spotNum, response);

    // We also add the externalId in the Bid extension field
    const Auction::Data *data = auction.getCurrentData();

    auto &resp = data->winningResponse(spotNum);
    const auto &agentConfig = resp.agentConfig;

    OpenRTB::SeatBid &seatBid = response.seatbid.back();

    OpenRTB::Bid &bid = seatBid.bid.back();

    Json::Value ext(Json::objectValue);
    ext["external-id"] = agentConfig->externalId;
    bid.ext = ext;
}


namespace {

struct Init
{ 
    Init()
    {
        RTBKIT::FilterRegistry::registerFilter<RTBKIT::ExternalIdsCreativeExchangeFilter>();
        RTBKIT::ExchangeConnector::registerFactory<RTBKIT::RTBKitExchangeConnector>();
    }
} init;

}

} // namespace RTBKIT
